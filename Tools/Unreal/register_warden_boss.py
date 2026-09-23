"""Register Warden assets using separate native MCP requests (no outer transaction)."""
from datetime import datetime, timezone
import json
from pathlib import Path

import register_monster_pack_native as native
from native_mcp_client import UnrealMCP


DATA = Path(__file__).resolve().parents[2] / "docs" / "Bosses"


def main():
    native.DATA = DATA
    plan = json.loads((DATA / "warden.json").read_text(encoding="utf-8"))
    c = UnrealMCP()
    c.request("tools/call", {"name": "list_toolsets", "arguments": {}})
    for name in [native.ASSETS, native.OBJECTS, native.BLUEPRINTS, native.DT, native.TAGS, "EditorToolset.EditorAppToolset"]:
        c.tool("describe_toolset", {"toolset_name": name})
    assert not c.call("EditorToolset.EditorAppToolset", "IsPIERunning", {}), "Stop PIE first"
    tables = ["DT_GASSkillData", "DT_DiaMonsterTable", "DT_SpawnGroupTable"]
    before = {name: native.rows(c, name) for name in tables}
    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    native.write_json("tables-before-" + stamp + ".json", before)
    for table, desired, field in [(tables[0], plan["skills"], "skillId"), (tables[1], plan["monsters"], "monsterId")]:
        occupied = {row[field] for key, row in before[table].items() if key not in desired}
        for key, row in desired.items():
            assert row[field] not in occupied, "ID conflict: " + key
            assert key not in before[table] or before[table][key][field] == row[field], "Row conflict: " + key
    tags = ["SkillType.Character.BossMonster.Warden"] + ["GASData.CoolDown." + str(s["skillId"]) for s in plan["skills"].values()]
    existing_tags = c.call(native.TAGS, "ListTags", {"parentTag": ""})
    for tag in tags:
        if tag not in existing_tags:
            c.call(native.TAGS, "AddTag", {"tagName": tag, "tagSource": "", "comment": "Warden boss"})
    for a in plan["assets"]:
        if not c.call(native.ASSETS, "exists", {"path": a["path"]}):
            assert c.call(native.ASSETS, "duplicate", {"path": a["source"], "new_path": a["path"]})
    for change in plan["assetProperties"]:
        ref = {"refPath": change["path"]}
        schema = json.loads(c.call(native.OBJECTS, "list_properties", {"instance": ref}))
        assert all(k in schema for k in change["values"]), "Property schema mismatch: " + change["path"]
        assert c.call(native.OBJECTS, "set_properties", {"instance": ref, "values": json.dumps(change["values"])})
    for a in plan["assets"]:
        if a["blueprint"]:
            c.call(native.BLUEPRINTS, "compile_blueprint", {"blueprint": native.asset_ref(a["path"]), "warnings_as_errors": True})
            print("Compiled " + a["path"], flush=True)
    for change in plan["assetProperties"]:
        actual = json.loads(c.call(native.OBJECTS, "get_properties", {"instance": {"refPath": change["path"]}, "properties": list(change["values"])}))
        native.verify_subset(actual, change["values"], change["path"])
    paths = [a["path"] for a in plan["assets"]]
    native.save(c, paths)
    after = {}
    for table, desired in zip(tables, [plan["skills"], plan["monsters"], plan["spawnGroups"]]):
        after[table] = native.write_table(c, table, desired, before[table])
    native.write_json("tables-after.json", after)
    native.write_json("registration-result.json", {"assets": paths, "tables": tables,
        "blueprints_compiled_with_warnings_as_errors": True,
        "existing_rows_preserved_except_boss_spawn_selection": True,
        "boss_group": "TestRoomBoss", "monster_id": "Warden_Boss", "skill_ids": [3201, 3202, 3203, 3204]})
    print("Warden registration and readback completed.")


if __name__ == "__main__":
    main()
