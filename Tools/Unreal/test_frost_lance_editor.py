"""Read-only Frost Lance smoke test for Unreal's execute_tool_script tool.

Pass this file's text as the 'script' argument to:
editor_toolset.toolsets.programmatic.ProgrammaticToolset.execute_tool_script
The editor provides execute_tool; this is not a standalone Python program.
"""
import json


def rows(table, names):
    result = execute_tool(
        "editor_toolset.toolsets.data_table.DataTableTools.get_rows",
        json.dumps({"data_table": {"refPath": "/Game/Datatable/" + table + "." + table},
                    "row_names": names}))
    return json.loads(result["returnValue"])


def properties(path, names):
    result = execute_tool(
        "editor_toolset.toolsets.object.ObjectTools.get_properties",
        json.dumps({"instance": {"refPath": path}, "properties": names}))
    return json.loads(result["returnValue"])


def asset_exists(path):
    return execute_tool(
        "editor_toolset.toolsets.asset.AssetTools.exists",
        json.dumps({"path": path}))["returnValue"]


def compile_state(path):
    return execute_tool(
        "NiagaraToolsets.NiagaraToolset_System.GetSystemCompileState",
        json.dumps({"system": {"refPath": path}}))["returnValue"]


def run():
    checks = []

    def check(name, actual, expected):
        checks.append({"name": name, "passed": actual == expected,
                       "actual": actual, "expected": expected})

    root = "/Game/Skills/FrostLance"
    skill = rows("DT_GASSkillData", ["FrostLance"])["FrostLance"]
    check("skill_id", skill["skillId"], 1012)
    check("original_variant_ids_preserved", all(v in skill["variantIds"] for v in [21, 22, 23]), True)
    check("unique_variant_ids", len(skill["variantIds"]), len(set(skill["variantIds"])))
    check("base_damage", skill["baseDamage"], 85)
    check("mana_cost", skill["manaCost"], 12)
    check("cooldown_seconds", skill["cooldownDuration"], 2)
    check("cooldown_tag", skill["coolDownTags"]["TagName"], "GASData.CoolDown.1012")
    projectile = skill["steps"][0]["ProjectileData"]
    check("spawn_class", projectile["SkillActorClass"].split("'")[1],
          root + "/BP_FrostLance.BP_FrostLance_C")
    check("lifetime_seconds", projectile["LifeSpan"], 1.5)
    job = rows("DT_JobSkillSet", ["WarriorSkillSet"])["WarriorSkillSet"]
    check("job_reward_pool", job["skillIds"].count(1012), 1)

    names = ["FrostLance_Pierce", "FrostLance_Fork", "FrostLance_Freeze"]
    variants = rows("DT_SkillVariantion", names)
    for name, variant_id, magnitude in zip(names, [21, 22, 23], [3, 2, 1.2]):
        variant = variants[name]
        check(name + ".id", variant["variantId"], variant_id)
        check(name + ".magnitude", abs(variant["modifierValue"] - magnitude) < 0.0001, True)
        check(name + ".icon", asset_exists(variant["icon"]), True)
    freeze_extra = variants["FrostLance_Freeze"]["variantExtraData"]
    check("freeze_effect_class", freeze_extra["TargetEffects"][0]["EffectClass"].split("'")[1],
          root + "/GE_FrostLance_Freeze.GE_FrostLance_Freeze_C")

    bp = properties(root + "/BP_FrostLance.BP_FrostLance",
                    ["projectileSpeed", "lifeSpan", "skillEffect", "hitEffect"])
    check("projectile_speed", bp["projectileSpeed"], 2000)
    for field, suffix in [("skillEffect", "Trail"), ("hitEffect", "Impact")]:
        path = root + "/Effects/NS_FrostLance_" + suffix
        check(field + ".reference", bp[field]["refPath"], path + ".NS_FrostLance_" + suffix)
        state = compile_state(path + ".NS_FrostLance_" + suffix)
        check(suffix + ".compile_status", state["aggregateStatus"], "UpToDate")
        check(suffix + ".compile_errors", state["bHasErrors"], False)
        check(suffix + ".compile_warnings", state["bHasWarnings"], False)

    mesh = properties(root + "/BP_FrostLance.Default__BP_FrostLance_C:SkillObjectMesh",
                      ["staticMesh"])
    check("projectile_mesh", mesh["staticMesh"]["refPath"],
          root + "/Meshes/SM_FrostLance.SM_FrostLance")
    check("projectile_mesh_exists", asset_exists(mesh["staticMesh"]["refPath"]), True)
    effect = properties(root + "/GE_FrostLance_Freeze.GE_FrostLance_Freeze",
                        ["durationPolicy", "durationMagnitude", "modifiers"])
    check("freeze_duration_policy", effect["durationPolicy"], "HasDuration")
    duration = effect["durationMagnitude"]["scalableFloatMagnitude"]["value"]
    check("freeze_duration_1_2s", abs(duration - 1.2) < 0.0001, True)
    modifiers = effect["modifiers"]
    check("freeze_movement_override", any(
        m["attribute"]["attributeName"] == "MovementSpeed"
        and m["modifierOp"] == "Override"
        and m["modifierMagnitude"]["scalableFloatMagnitude"]["value"] == 0
        for m in modifiers), True)
    tags = properties(root + "/GE_FrostLance_Freeze.Default__GE_FrostLance_Freeze_C:TargetTags",
                      ["inheritableGrantedTagsContainer"])
    granted = tags["inheritableGrantedTagsContainer"]["combinedTags"]["gameplayTags"]
    check("freeze_grants_required_tags",
          {"GASData.State.Stunned", "GASData.State.Freeze"}.issubset(
              {tag["tagName"] for tag in granted}), True)

    # Regression guard for the editor table tool's InstancedStruct TMap round-trip.
    slow = rows("DT_SkillVariantion", ["SlowHit"])["SlowHit"]
    values = slow["variantExtraData"]["TargetEffects"][0]["SetByCallerMagnitudes"]
    for tag, expected in [("GASData.Duration", 2), ("GASData.StatusValue", 0.1)]:
        value = values.get('(TagName="' + tag + '")')
        if isinstance(value, dict):
            value = value.get("SetByCallerMagnitudes")
        check("existing_slow." + tag, value, expected)

    return {"suite": "FrostLance.EditorSmoke", "passed": all(c["passed"] for c in checks),
            "total": len(checks), "checks": checks,
            "scope": "Editor data and asset configuration only; no combat or modified C++ execution."}
