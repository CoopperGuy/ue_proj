"""Repair monster combat assets with native MCP and separate DataTable writes."""
import copy
import json
from pathlib import Path

from native_mcp_client import UnrealMCP
from register_monster_pack_native import (
    ASSETS, OBJECTS, BLUEPRINTS, DT, TAGS, asset_ref, rows, save, write_table,
)

OUT = Path(__file__).resolve().parents[2] / "docs" / "CombatFix"
BT = "aimodule_toolset.toolsets.behavior_tree.BehaviorTreeTools"
MONTAGE = "VibeUE.AnimMontageService"
GRAPH = "VibeUE.AnimGraphService"
BP = "VibeUE.BlueprintService"


def melee(radius, angle=110, offset=35, cosmetic="None", delay=0):
    return {"_structType": "/Script/ARPG.GASMeleeSpawnStepData",
            "MeleeData": {"AttackRange": radius, "AttackAngle": angle,
                          "AttackOffset": {"X": offset, "Y": 0, "Z": 0},
                          "bShowDebugShape": False, "SkillActorClass": cosmetic, "LifeSpan": 0.8},
            "bUseLastStepEndLocation": False, "StartDelay": delay, "bWaitUntilFinished": True}


def main():
    c = UnrealMCP()
    for toolset in (ASSETS, OBJECTS, BLUEPRINTS, DT, TAGS, BT, MONTAGE, GRAPH, BP, "EditorToolset.EditorAppToolset"):
        c.tool("describe_toolset", {"toolset_name": toolset})
    assert not c.call("EditorToolset.EditorAppToolset", "IsPIERunning", {})
    before = {n: rows(c, n) for n in ("DT_DiaMonsterTable", "DT_GASSkillData")}
    assets, changes = [], []

    def props(ref, values):
        schema = json.loads(c.call(OBJECTS, "list_properties", {"instance": ref}))
        assert all(k in schema for k in values), (ref, values)
        old = json.loads(c.call(OBJECTS, "get_properties", {"instance": ref, "properties": list(values)}))
        assert c.call(OBJECTS, "set_properties", {"instance": ref, "values": json.dumps(values)})
        actual = json.loads(c.call(OBJECTS, "get_properties", {"instance": ref, "properties": list(values)}))
        changes.append({"ref": ref, "before": old, "after": actual})

    generic = "/Game/Blueprint/Monster/AI/BT_DiaMonsterTree"
    new_trees = {}
    for role, skill, radius in (("Raptor", 3003, 280), ("Bomber", 3002, 240)):
        tree = f"/Game/Blueprint/Monster/{role}/AI/BT_{role}Combat"
        if not c.call(ASSETS, "exists", {"path": tree}):
            assert c.call(ASSETS, "duplicate", {"path": generic, "new_path": tree})
        for ref in c.call(BT, "list_nodes", {"behavior_tree": asset_ref(tree)}):
            name = ref["refPath"].split(":")[-1]
            if name.startswith(("BTDecorator_IsValidSkill", "BTTask_RotateToTarget")):
                props(ref, {"skillId": skill})
            if name.startswith("UBTD_DistanceInRange"):
                props(ref, {"minDistance": 0, "maxDistance": radius})
        new_trees[role] = tree
        assets.append(tree)

    # Cooldown checks belong to selection/activation. An outstanding request must
    # reach PlayGAS so failed and interrupted requests can always be consumed.
    trees = {r["behaviorTree"].split(".")[0] for r in before["DT_DiaMonsterTable"].values()} | set(new_trees.values())
    for tree in sorted(trees):
        for ref in c.call(BT, "list_nodes", {"behavior_tree": asset_ref(tree)}):
            if ":BTDecorator_CheckActionRequest_" in ref["refPath"]:
                props(ref, {"bCheckCooldown": False})
        assets.append(tree)

    animations = [
        ("Raptor", "/Game/ParagonMinions/Characters/Buff/Buff_Black/Animations/BiteAttack_A",
         "/Game/Blueprint/Monster/Animation/BP_DiaInstance", "AnimGraphNode_UseCachedPose"),
        ("zombie", "/Game/Maximo/Zombie/Animation/Zombie_Attack",
         "/Game/Blueprint/Monster/zombie/Animation/BP_DiaZombieAnimInstance", "AnimGraphNode_StateMachine"),
    ]
    montages = {}
    for role, sequence, anim_bp, source_type in animations:
        folder = f"/Game/Blueprint/Monster/{role}/Animation"
        name = "AM_RaptorBite" if role == "Raptor" else "AM_ZombieAttack"
        montage = folder + "/" + name
        if not c.call(ASSETS, "exists", {"path": montage}):
            assert c.call(MONTAGE, "CreateMontageFromAnimation", {
                "animSequencePath": sequence, "destPath": folder, "montageName": name})
        nodes = c.call(BP, "GetNodesInGraph", {"blueprintPath": anim_bp, "graphName": "AnimGraph"})
        slots = [n for n in nodes if n["nodeType"] == "AnimGraphNode_Slot"]
        if not slots:
            source = next(n for n in nodes if n["nodeType"] == source_type)
            slot = c.call(GRAPH, "AddSlotNode", {"animBlueprintPath": anim_bp, "graphName": "AnimGraph", "slotName": "DefaultSlot", "posX": -180, "posY": 0})
            assert slot
            assert c.call(GRAPH, "ConnectAnimNodes", {"animBlueprintPath": anim_bp, "graphName": "AnimGraph",
                "sourceNodeId": source["nodeId"], "sourcePinName": "Pose", "targetNodeId": slot, "targetPinName": "Source"})
        else:
            slot = slots[0]["nodeId"]
        # The native AnimGraph service uses MakeLinkTo, which does not replace
        # the existing input connection. Disconnect it before inserting a slot.
        root = next(n for n in nodes if n["nodeType"] == "AnimGraphNode_Root")
        assert c.call(GRAPH, "DisconnectAnimNode", {"animBlueprintPath": anim_bp, "graphName": "AnimGraph",
            "nodeId": root["nodeId"], "pinName": "Result"})
        assert c.call(GRAPH, "ConnectToOutputPose", {"animBlueprintPath": anim_bp, "graphName": "AnimGraph", "sourceNodeId": slot})
        if role == "Raptor":
            # Both existing locomotion transition rules were disconnected.
            for source, dest, comparison in (("Idle", "Move", "greater"), ("Move", "Idle", "less_equal")):
                assert c.call(GRAPH, "SetTransitionRuleComparison", {"animBlueprintPath": anim_bp,
                    "stateMachineName": "Locomotion", "sourceStateName": source, "destStateName": dest,
                    "floatVariableName": "Speed", "comparison": comparison, "threshold": 5})
        c.call(BLUEPRINTS, "compile_blueprint", {"blueprint": asset_ref(anim_bp), "warnings_as_errors": True})
        montages[role] = montage + "." + name
        assets.extend([montage, anim_bp])

    blast = "/Game/Blueprint/Monster/Bomber/Skills/BP_BomberBlast"
    if not c.call(ASSETS, "exists", {"path": blast}):
        assert c.call(ASSETS, "duplicate", {"path": "/Game/Blueprint/SkillObj/BP_Explosion", "new_path": blast})
    props(asset_ref(blast), {"damage": 0, "lifeSpan": 0.8, "initialLifeSpan": 0.8})
    props({"refPath": blast + ".Default__BP_BomberBlast_C:SphereComp"}, {"bodyInstance": {"collisionEnabled": "NoCollision"}})
    c.call(BLUEPRINTS, "compile_blueprint", {"blueprint": asset_ref(blast), "warnings_as_errors": True})
    assets.append(blast)
    save(c, list(dict.fromkeys(assets)))

    skills = {
        "ZombieBullet": {"castAnimation": montages["zombie"], "castTime": 0.45,
                         "cooldownDuration": 2.4, "skillType": "MeleeAttack", "steps": [melee(220)]},
        "BomberBomb": {"cooldownDuration": 4.5, "steps": [melee(260, 360, 0,
            "/Script/Engine.BlueprintGeneratedClass'" + blast + ".BP_BomberBlast_C'")]},
    }
    raptor = copy.deepcopy(before["DT_GASSkillData"]["BladeGuardSlash"])
    raptor.update({"skillId": 3003, "skillName": "Raptor Bite", "description": "Bite the target after a short wind-up.",
                   "castAnimation": montages["Raptor"], "castTime": 0.45, "cooldownDuration": 2,
                   "baseDamage": 14, "coolDownTags": {"TagName": "GASData.CoolDown.3003"}, "steps": [melee(250, 120, 40)]})
    skills["RaptorBite"] = raptor
    # The retained old boss has a charge step with no damage step.
    charge = copy.deepcopy(before["DT_GASSkillData"]["DualBossSKill_03"]["steps"])
    if not any(s.get("_structType") == "/Script/ARPG.GASMeleeSpawnStepData" for s in charge):
        charge.append(melee(220, 180, 0))
    skills["DualBossSKill_03"] = {"steps": charge}

    tags = copy.deepcopy(before["DT_DiaMonsterTable"]["BladeGuard_Normal"]["monsterTags"])
    monsters = {key: {"monsterSkills": [3003], "monsterTags": tags,
        "behaviorTree": new_trees["Raptor"] + ".BT_RaptorCombat", "attackRange": 200}
        for key in ("Raptor_Normal", "Raptor_Elite")}
    monsters["Bomber_Normal"] = {"monsterTags": tags,
        "behaviorTree": new_trees["Bomber"] + ".BT_BomberCombat", "attackRange": 180}
    if "GASData.CoolDown.3003" not in c.call(TAGS, "ListTags", {"parentTag": "GASData.CoolDown"}):
        c.call(TAGS, "AddTag", {"tagName": "GASData.CoolDown.3003", "tagSource": "", "comment": "Raptor bite cooldown"})
    after = {"DT_GASSkillData": write_table(c, "DT_GASSkillData", skills, before["DT_GASSkillData"]),
             "DT_DiaMonsterTable": write_table(c, "DT_DiaMonsterTable", monsters, before["DT_DiaMonsterTable"])}
    (OUT / "asset-repair.json").write_text(json.dumps({"assets": assets, "propertyChanges": changes, "tables": after}, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps({"savedAssets": len(assets), "repairedSkills": list(skills), "repairedMonsters": list(monsters)}))


if __name__ == "__main__":
    main()
