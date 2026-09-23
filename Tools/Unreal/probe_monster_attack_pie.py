"""Isolate an already spawned monster and record an attack through native MCP.

Use only in a disposable PIE session. Stop PIE afterwards to discard test positions
and paused AI. This does not modify assets, tables, or save slots.
"""
import argparse
import json
from pathlib import Path
import time

from native_mcp_client import UnrealMCP


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("actor", help="Exact active monster name from the PIE snapshot")
    parser.add_argument("skill", type=int)
    parser.add_argument("distance", type=float)
    parser.add_argument("--ai", action="store_true", help="Let the behavior tree chase and choose attacks")
    parser.add_argument("--center-x", type=float, default=5000)
    parser.add_argument("--center-y", type=float, default=4000)
    parser.add_argument("--interrupt-after", type=float, help="Stop the attack montage after this many game seconds")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    client = UnrealMCP()
    gas = "GASToolsets.AbilitySystemInspectorToolset"
    client.tool("describe_toolset", {"toolset_name": gas})
    code = '''import unreal, json
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert world is not None
player = unreal.GameplayStatics.get_player_pawn(world, 0)
monsters = [m for m in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.DiaMonster)
            if not m.get_editor_property("hidden") and m.get_controller()]
'''
    code += "monster = next(m for m in monsters if m.get_name() == " + repr(args.actor) + ")\n"
    code += '''for index, other in enumerate(monsters):
    controller = other.get_controller()
    brain = controller.get_component_by_class(unreal.BrainComponent)
    if brain:
        brain.stop_logic("Monster pack isolated attack check")
    controller.stop_movement()
    if other != monster:
        other.set_actor_location(unreal.Vector(3900 + index * 130, 4400, other.get_actor_location().z), False, True)
'''
    code += f"player.set_actor_location(unreal.Vector({args.center_x}, {args.center_y}, 90.15), False, True)\n"
    code += f"monster.set_actor_location(unreal.Vector({args.center_x - args.distance}, {args.center_y}, monster.get_actor_location().z), False, True)\n"
    code += '''monster.set_actor_rotation(unreal.Rotator(0, 0, 0), True)
bb = monster.get_controller().get_component_by_class(unreal.BlackboardComponent)
bb.set_value_as_object("TargetActor", player)
unreal._monster_attack_probe = {"actor": monster.get_path_name(), "player": player.get_path_name(), "samples": [], "elapsed": 0.0,
                              "started_at": unreal.GameplayStatics.get_time_seconds(world)}
player_asc = player.get_editor_property("ability_system_component")
health_attribute = next(a for a in player_asc.get_all_attributes() if a.get_editor_property("attribute_name") == "Health")
def sample_tick(dt, actor=monster, world=world, state=unreal._monster_attack_probe,
                player_asc=player_asc, health_attribute=health_attribute):
    state["elapsed"] += dt
    location = actor.get_actor_location()
    anim = actor.mesh.get_anim_instance()
    montage = anim.get_current_active_montage() if anim else None
    interrupt_after = state.get("interrupt_after")
    if interrupt_after is not None and not state.get("interrupted") and montage:
        if unreal.GameplayStatics.get_time_seconds(world) - state["activated_at"] >= interrupt_after:
            anim.montage_stop(0.05, montage)
            state["interrupted"] = True
    projectiles = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.DiaProjectile)
    blackboard = actor.get_controller().get_component_by_class(unreal.BlackboardComponent)
    state["samples"].append({"time": round(state["elapsed"], 3),
        "game_time": round(unreal.GameplayStatics.get_time_seconds(world) - state["started_at"], 3),
        "action_request": blackboard.get_value_as_int("ActionRequest") if blackboard else None,
        "phase": anim.get_editor_property("phase") if isinstance(anim, unreal.DiaMonInstance) else None,
        "player_health": player_asc.get_gameplay_attribute_value(health_attribute)[0],
        "location": [round(location.x, 2), round(location.y, 2), round(location.z, 2)],
        "montage": montage.get_path_name() if montage else None,
        "projectiles": [p.get_name() for p in projectiles]})
    if state["elapsed"] >= 5:
        unreal.unregister_slate_post_tick_callback(state.pop("handle"))
unreal._monster_attack_probe["handle"] = unreal.register_slate_post_tick_callback(sample_tick)
print(json.dumps({"actor": monster.get_path_name(), "player": player.get_path_name()}))
'''
    setup = client.tool("execute_python_code", {"code": code})
    paths = json.loads(setup["output"])
    player_ref = {"actor": {"refPath": paths["player"]}}
    actor_ref = {"actor": {"refPath": paths["actor"]}}
    before = client.call(gas, "GetAttributeValues", player_ref)
    activate_code = (
        "import unreal,json\n"
        "m=unreal.find_object(None," + repr(paths["actor"]) + ")\n"
    )
    if args.interrupt_after is not None:
        activate_code += f"unreal._monster_attack_probe['interrupt_after'] = {args.interrupt_after!r}\n"
        activate_code += "unreal._monster_attack_probe['activated_at'] = unreal.GameplayStatics.get_time_seconds(unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world())\n"
    if args.ai:
        activate_code += "m.get_controller().get_component_by_class(unreal.BrainComponent).restart_logic()\nprint(json.dumps({'ai_restarted': True}))"
    else:
        activate_code += f"print(json.dumps({{'activated': m.get_editor_property('skill_manager_component').try_activate_ability_by_skill_id({args.skill})}}))"
    activation = client.tool("execute_python_code", {"code": activate_code})
    time.sleep(14 if args.ai else 6)
    after = client.call(gas, "GetAttributeValues", player_ref)
    abilities = client.call(gas, "GetGrantedAbilities", actor_ref)
    probe = client.tool("execute_python_code", {"code":
        "import unreal,json\n"
        "s=unreal._monster_attack_probe\n"
        "if 'handle' in s: unreal.unregister_slate_post_tick_callback(s.pop('handle'))\n"
        "print(json.dumps(s))"})
    result = json.loads(probe["output"])
    result["activation"] = json.loads(activation["output"])
    result["skill"] = args.skill
    result["health_before"] = next(r["currentValue"] for r in before if r["attributeName"] == "Health")
    result["health_after"] = next(r["currentValue"] for r in after if r["attributeName"] == "Health")
    result["abilities_after"] = abilities
    args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({k: v for k, v in result.items() if k != "samples"}))
    print(json.dumps({"samples": len(result["samples"]),
        "montages": sorted({s["montage"] for s in result["samples"] if s["montage"]}),
        "projectiles": sorted({p for s in result["samples"] for p in s["projectiles"]}),
        "first_location": result["samples"][0]["location"],
        "last_location": result["samples"][-1]["location"]}))


if __name__ == "__main__":
    main()
