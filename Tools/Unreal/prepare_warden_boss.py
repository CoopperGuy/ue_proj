"""Prepare the Warden manifest from the native MCP inspection snapshot."""
import copy
import json
from pathlib import Path

DATA = Path(__file__).resolve().parents[2] / "docs" / "Bosses"
before = json.loads((DATA / "inspected-before.json").read_text(encoding="utf-8"))
normal = json.loads((DATA.parent / "Monsters" / "pack.json").read_text(encoding="utf-8"))
ROOT = "/Game/Blueprint/Monster/Boss/Warden"
OLD = "/Game/Blueprint/Monster/Boss/DualBladeBoss"


def obj(path):
    return path + "." + path.rsplit("/", 1)[1]


def cls(path):
    return "/Script/Engine.BlueprintGeneratedClass'" + obj(path) + "_C'"


plan = {"label": "쌍검 수문장", "assets": [], "assetProperties": [], "skills": {}, "monsters": {}}


def duplicate(source, path, blueprint=False):
    plan["assets"].append({"source": source, "path": path, "blueprint": blueprint})


def properties(path, **values):
    plan["assetProperties"].append({"path": path, "values": values})


bp = ROOT + "/BP_WardenBoss"
bt = ROOT + "/AI/BT_WardenBoss"
bb = ROOT + "/AI/BB_WardenBoss"
duplicate(OLD + "/BP_DualBladeBoss", bp, True)
duplicate(OLD + "/AI/BT_DualBladeBoss", bt)
duplicate(OLD + "/AI/BB_DualBladeBB", bb)
projectile = ROOT + "/Skills/BP_WardenBladeWave"
impact = ROOT + "/Skills/BP_WardenImpact"
duplicate("/Game/Blueprint/Character/Skill/Object/BP_DBBossSlash", projectile, True)
duplicate("/Game/Blueprint/SkillObj/BP_GroundImpact", impact, True)
properties(obj(projectile), projectileSpeed=700, lifeSpan=3, initialLifeSpan=3)
properties(obj(impact), radius=360, damage=0, lifeSpan=0.8, initialLifeSpan=0.8)
properties(impact + ".Default__BP_WardenImpact_C:SphereComp",
           bodyInstance={"collisionEnabled": "NoCollision"})
properties(impact + ".Default__BP_WardenImpact_C:SkillEffectComp", relativeScale3D={"x": 1.8, "y": 1.8, "z": 1.8})
boss_tag = "SkillType.Character.BossMonster.Warden"
properties(obj(bp), tags=["Character", "Actor.Monster"],
           abilityTags={"gameplayTags": [{"tagName": boss_tag}]},
           aIControllerClass={"refPath": "/Game/Blueprint/Monster/AI/BP_DiaAIController.BP_DiaAIController_C"})
properties(bp + ".Default__BP_WardenBoss_C:CharacterMesh0",
           relativeScale3D={"x": 1.2, "y": 1.2, "z": 1.2}, relativeLocation={"x": 0, "y": 0, "z": -120})
properties(bp + ".Default__BP_WardenBoss_C:CollisionCylinder", capsuleHalfHeight=120, capsuleRadius=72)
properties(obj(bt), blackboardAsset={"refPath": obj(bb)})


def skill(row, skill_id, title, description, cast, cooldown, damage, source_montage, rate):
    value = copy.deepcopy(normal["skills"]["BladeGuardSlash"])
    montage = ROOT + "/Animation/AM_" + row
    duplicate(OLD + "/Animation/Montage/" + source_montage, montage)
    properties(obj(montage), rateScale=rate)
    value.update(skillId=skill_id, skillName=title, description=description, castTime=cast,
                 cooldownDuration=cooldown, baseDamage=damage, castAnimation=obj(montage), steps=[])
    value["abilityTags"] = {"GameplayTags": [{"TagName": boss_tag}], "ParentTags": [
        {"TagName": t} for t in ["SkillType.Character.BossMonster", "SkillType.Character", "SkillType"]]}
    value["coolDownTags"] = {"TagName": "GASData.CoolDown." + str(skill_id)}
    plan["skills"][row] = value
    return value


def melee(radius, angle, offset=0, delay=0, cosmetic="None"):
    value = copy.deepcopy(normal["skills"]["BladeGuardSlash"]["steps"][0])
    value["StartDelay"] = delay
    value["MeleeData"].update(AttackRange=radius, AttackAngle=angle, AttackOffset={"X": offset, "Y": 0, "Z": 0},
                              SkillActorClass=cosmetic, LifeSpan=0.8 if cosmetic != "None" else 0)
    return value


s = skill("WardenCrossCut", 3201, "교차 베기", "준비 동작 후 정면을 두 번 벱니다.", 0.6, 3.2, 20, "AM_DualBossSlash", 1.0)
s["steps"] = [melee(270, 110, 45), melee(270, 110, 45, 0.4)]
s = skill("WardenCharge", 3202, "수문장의 돌진", "0.85초 준비 후 500cm 돌진하고 도착 지점을 공격합니다.", 0.85, 5.5, 30, "AM_DualBossDash", 0.4)
s["steps"] = copy.deepcopy(normal["skills"]["ChargerRush"]["steps"])
s["steps"][0]["ChargeData"]["ChargeDuration"] = 0.5
s["steps"][1] = melee(260, 180)
s = skill("WardenSlam", 3203, "원형 강타", "0.9초 준비 후 주위 360cm를 한 번 공격합니다.", 0.9, 4, 32, "AM_DualBossJumpDash", 0.6)
s["steps"] = [melee(360, 360, cosmetic=cls(impact))]
s = skill("WardenBladeFan", 3204, "황혼의 검기", "0.75초 준비 후 부채꼴로 검기 다섯 발을 발사합니다.", 0.75, 3.5, 24, "AM_DualBossSlash", 1.0)
s["skillType"] = "RangedAttack"
s["steps"] = copy.deepcopy(normal["skills"]["SpitterBolt"]["steps"])
s["steps"][0]["ProjectileData"].update(ProjectileOffset={"X": 150, "Y": 0, "Z": 0},
    MinimumRange=150, bFireMultipleProjectiles=True, ProjectileCount=5, SpreadAngle=15,
    SkillActorClass=cls(projectile), LifeSpan=3)

monster = copy.deepcopy(before["DT_DiaMonsterTable"]["DualBlade_Boss"])
monster.update(monsterId="Warden_Boss", maxHP=1600, maxMP=0, attack=18, defense=8, exp=100,
               attackRange=220, monsterSkills=[3201, 3202, 3203, 3204], behaviorTree=obj(bt), blackboardAsset=obj(bb),
               bP_MonsterAsset=cls(bp), monsterTags=copy.deepcopy(s["abilityTags"]))
plan["monsters"]["Warden_Boss"] = monster

id_map = {2001: 3201, 2003: 3202, 2002: 3203, 2005: 3204}
distances = {"UBTD_DistanceInRange_0": (0, 300), "UBTD_DistanceInRange_2": (300, 800),
             "UBTD_DistanceInRange_8": (0, 360), "UBTD_DistanceInRange_1": (360, 1300)}
node_labels = {"BTComposite_Selector_3": "Choose health phase", "BTComposite_Selector_6": "Phase 1: HP >= 50%",
               "BTComposite_Selector_4": "Phase 2: HP < 50%", "BTComposite_Sequence_13": "Cross cut: two hits",
               "BTComposite_Sequence_11": "Charge and impact", "BTComposite_Sequence_2": "Circular slam",
               "BTComposite_Sequence_3": "Five-blade fan", "BTComposite_Sequence_6": "Cast, then recover"}
for path, node in before["tree"].items():
    values = node["values"]
    new_path = path.replace(OLD + "/AI/BT_DualBladeBoss.BT_DualBladeBoss", obj(bt))
    name = path.rsplit(":", 1)[1]
    if name in node_labels:
        properties(new_path, nodeName=node_labels[name])
    if values.get("skillId") in id_map:
        properties(new_path, skillId=id_map[values["skillId"]])
    if name in distances:
        low, high = distances[name]
        properties(new_path, minDistance=low, maxDistance=high)
    if name == "BTTask_Wait_2":
        properties(new_path, waitTime={"defaultValue": 1.1, "key": "None"}, nodeName="Recovery (1.1s)")
    if name in ["BTTask_Wait_0", "BTTask_Wait_3"]:
        properties(new_path, waitTime={"defaultValue": 0.15, "key": "None"}, randomDeviation={"defaultValue": 0, "key": "None"})

plan["spawnGroups"] = {"NewRow_0": {"monsterSpawnInfos": [{"MonsterID": "Warden_Boss", "SpawnLevel": 1, "SpawnWeight": 1}]}}
(DATA / "warden.json").write_text(json.dumps(plan, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
print("Prepared Warden: 4 skills, 2 phases, 9 assets")
