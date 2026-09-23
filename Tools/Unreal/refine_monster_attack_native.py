"""Align monster attack assets and active hit windows through native Unreal MCP."""
import copy
import json
from pathlib import Path
from native_mcp_client import UnrealMCP
from register_monster_pack_native import ASSETS, DT, OBJECTS, asset_ref, rows, save, write_table

OUT = Path(__file__).resolve().parents[2] / "docs" / "CombatFix"
MONTAGE = "VibeUE.AnimMontageService"
APP = "EditorToolset.EditorAppToolset"
BT = "aimodule_toolset.toolsets.behavior_tree.BehaviorTreeTools"

def main():
    c = UnrealMCP()
    for group in (ASSETS, DT, OBJECTS, MONTAGE, APP, BT):
        c.tool("describe_toolset", {"toolset_name": group})
    assert not c.call(APP, "IsPIERunning", {})
    before = rows(c, "DT_GASSkillData")
    assets = []
    paths = {}
    sequences = {
        "BladeGuard": "/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Super/Attack_A",
        "Bomber": "/Game/ParagonMinions/Characters/Buff/Buff_White/Animations/Melee_Attack_01_A",
        "Spitter": "/Game/ParagonMinions/Characters/Buff/Buff_White/Animations/Melee_Attack_01_A",
    }
    for role, sequence in sequences.items():
        folder = f"/Game/Blueprint/Monster/{role}/Animation"
        name = f"AM_{role}Attack"
        path = folder + "/" + name
        if not c.call(ASSETS, "exists", {"path": path}):
            assert c.call(MONTAGE, "CreateMontageFromAnimation", {"animSequencePath": sequence, "destPath": folder, "montageName": name})
        paths[role] = path + "." + name
        assets.append(path)
    fan = "/Game/Blueprint/Monster/Boss/Warden/Animation/AM_WardenBladeFan"
    segments = c.call(MONTAGE, "ListAnimSegments", {"montagePath": fan, "trackIndex": 0})
    assert len(segments) in (1, 2)
    if len(segments) == 2:
        assert c.call(MONTAGE, "RemoveAnimSegment", {"montagePath": fan, "trackIndex": 0, "segmentIndex": 1})
    assets.append(fan)
    # Selection distance must stay inside the capsule contact reach of each attack.
    range_changes = []
    for path, radius in (
        ("/Game/Blueprint/Monster/Raptor/AI/BT_RaptorCombat", 235),
        ("/Game/Blueprint/Monster/AI/BT_DiaMonsterTree", 170),
        ("/Game/Blueprint/Monster/BladeGuard/AI/BT_DiaBladeGuard", 205),
        ("/Game/Blueprint/Monster/Charger/AI/BT_DiaCharger", 205),
    ):
        for ref in c.call(BT, "list_nodes", {"behavior_tree": asset_ref(path)}):
            if ":UBTD_DistanceInRange_" not in ref["refPath"]: continue
            schema = json.loads(c.call(OBJECTS, "list_properties", {"instance": ref}))
            assert "maxDistance" in schema
            old = json.loads(c.call(OBJECTS, "get_properties", {"instance": ref, "properties": ["minDistance", "maxDistance"]}))
            field = "minDistance" if old["minDistance"] > 0 else "maxDistance"
            assert old["minDistance"] == 0 or "Charger" in path
            assert c.call(OBJECTS, "set_properties", {"instance": ref, "values": json.dumps({field: radius})})
            new = json.loads(c.call(OBJECTS, "get_properties", {"instance": ref, "properties": [field]}))
            assert new[field] == radius
            range_changes.append({"node": ref, "before": old, "after": new})
        assets.append(path)
    save(c, assets)

    patches = {}
    for key, radius, angle, offset, window in (
        ("ZombieBullet", 135, 100, 15, 0.18),
        ("RaptorBite", 190, 100, 25, 0.14),
        ("BladeGuardSlash", 165, 100, 20, 0.15),
        ("WardenCrossCut", 250, 110, 25, 0.15),
        ("ChargerRush", 180, 360, 0, 0.12),
        ("WardenCharge", 260, 180, 0, 0.12),
    ):
        steps = copy.deepcopy(before[key]["steps"])
        for step in steps:
            if step["_structType"] == "/Script/ARPG.GASMeleeSpawnStepData":
                step["HitWindowDuration"] = window
                step["MeleeData"].update({"AttackRange": radius, "AttackAngle": angle,
                    "AttackOffset": {"X": offset, "Y": 0, "Z": 0}})
        patches[key] = {"steps": steps}
    patches["ZombieBullet"]["castTime"] = 1.02
    patches["BladeGuardSlash"]["castAnimation"] = paths["BladeGuard"]
    patches["ChargerRush"]["castTime"] = 0.2
    patches["WardenCharge"]["castTime"] = 0.2
    patches["WardenCrossCut"]["castTime"] = 0.4
    patches["WardenCrossCut"]["steps"][1]["StartDelay"] = 0.95
    # This retained two-swing montage uses the same two explicit hit windows.
    patches["DualBossSKill_01"] = {"steps": copy.deepcopy(patches["WardenCrossCut"]["steps"]), "castTime": 0.4}
    patches["BomberBomb"] = {"castAnimation": paths["Bomber"], "castTime": 0.3}
    patches["SpitterBolt"] = {"castAnimation": paths["Spitter"], "castTime": 0.3}
    patches["WardenBladeFan"] = {"castTime": 0.45}
    after = write_table(c, "DT_GASSkillData", patches, before)
    monster_before = rows(c, "DT_DiaMonsterTable")
    monster_after = write_table(c, "DT_DiaMonsterTable", {"Charger_Normal": {"attackRange": 180}}, monster_before)
    evidence = {"assets": assets, "rangeChanges": range_changes, "chargerBefore": monster_before["Charger_Normal"], "chargerAfter": monster_after["Charger_Normal"], "before": {k:before[k] for k in patches}, "after": {k:after[k] for k in patches}}
    previous = OUT / "attack-precision-assets.json"
    if previous.exists():
        evidence["before"] = json.loads(previous.read_text(encoding="utf-8"))["before"]
    (OUT / "attack-precision-assets.json").write_text(json.dumps(evidence, ensure_ascii=False, indent=2), encoding="utf-8")
    print(json.dumps({"savedAssets": len(assets)+2, "skills": list(patches)}))

if __name__ == "__main__":
    main()
