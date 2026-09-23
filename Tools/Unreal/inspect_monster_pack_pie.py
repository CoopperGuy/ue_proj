"""Inspect this task's PIE session; optionally enter a room through its trigger.

Run outside Unreal. All runtime access goes through the native Unreal MCP.
No level assets or save slots are written.
"""
import argparse
import json
from pathlib import Path

from native_mcp_client import UnrealMCP


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--enter", help="Exact room actor name from an earlier snapshot")
    parser.add_argument("--inside", action="store_true", help="Move inward after --enter reached the doorway")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    code = '''import unreal
import json
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert world is not None, "PIE is not running"
player = unreal.GameplayStatics.get_player_pawn(world, 0)
assert player is not None, "No player pawn"
rooms = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.DiaRoomBase)
'''
    if args.enter:
        code += "room = next(r for r in rooms if r.get_name() == " + repr(args.enter) + ")\n"
        code += '''triggers = room.get_editor_property("room_enter_triggers")
assert len(triggers) > 0, "Room has no entrance trigger"
door = next(t for t in triggers if t.get_collision_enabled() != unreal.CollisionEnabled.NO_COLLISION)
location = door.get_world_location()
'''
        if args.inside:
            code += "location = location - door.get_forward_vector() * 220.0\n"
        code += "player.set_actor_location(unreal.Vector(location.x, location.y, player.get_actor_location().z), False, True)\n"
    code += '''
def xyz(v):
    return [round(v.x, 1), round(v.y, 1), round(v.z, 1)]
monsters = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.DiaMonster)
print(json.dumps({
    "player": player.get_path_name(), "player_location": xyz(player.get_actor_location()),
    "rooms": [{"name": r.get_name(), "path": r.get_path_name(),
        "role": str(r.get_editor_property("room_role")),
        "state": str(r.get_editor_property("room_battle_state")),
        "location": xyz(r.get_actor_location()),
        "triggers": len(r.get_editor_property("room_enter_triggers"))} for r in rooms],
    "monsters": [{"name": m.get_name(), "path": m.get_path_name(),
        "class": m.get_class().get_path_name(), "location": xyz(m.get_actor_location()),
        "hidden": m.get_editor_property("hidden"),
        "tags": [str(t) for t in m.get_editor_property("tags")]} for m in monsters]
}))
'''
    client = UnrealMCP()
    result = client.tool("execute_python_code", {"code": code})
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
    if result.get("success") and result.get("output"):
        snapshot = json.loads(result["output"])
        snapshot["pooled_monster_count"] = sum(m["hidden"] for m in snapshot["monsters"])
        snapshot["monsters"] = [m for m in snapshot["monsters"] if not m["hidden"]]
        snapshot["rooms"] = [r for r in snapshot["rooms"] if "BATTLE_ACTIVE" in r["state"]]
        print(json.dumps(snapshot, ensure_ascii=False))
    else:
        print(json.dumps(result, ensure_ascii=False))


if __name__ == "__main__":
    main()
