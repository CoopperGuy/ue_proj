"""Apply test damage through GAS in disposable PIE, exercising real phase/death hooks."""
import argparse
import json
from pathlib import Path

from native_mcp_client import UnrealMCP


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("damage_base", type=float)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    c = UnrealMCP()
    code = '''import unreal,json
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
assert world is not None
player=unreal.GameplayStatics.get_player_pawn(world,0)
boss=next(m for m in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.DiaMonster)
          if m.get_name()=="BP_WardenBoss_C_0" and not m.get_editor_property("hidden"))
controller=boss.get_controller()
controller.get_component_by_class(unreal.BrainComponent).stop_logic("Boss phase test")
controller.stop_movement()
source=player.get_editor_property("ability_system_component")
target=boss.get_editor_property("ability_system_component")
spec=source.make_outgoing_spec(unreal.DiaGameplayEffect_Damage,1,source.make_effect_context())
tag=unreal.GameplayTag()
assert tag.import_text('(TagName="GASData.DamageBase")')
'''
    code += f"spec=unreal.AbilitySystemLibrary.assign_tag_set_by_caller_magnitude(spec,tag,{args.damage_base!r})\n"
    code += '''source.apply_gameplay_effect_spec_to_target(spec,target)
print(json.dumps({"boss":boss.get_path_name(),"hidden_after_damage":boss.get_editor_property("hidden"),
    "room_states":[{"name":r.get_name(),"state":str(r.get_editor_property("room_battle_state"))}
        for r in unreal.GameplayStatics.get_all_actors_of_class(world,unreal.DiaRoomBase)
        if "STAGE_CLEAR" in str(r.get_editor_property("room_role"))]}))
'''
    result = c.tool("execute_python_code", {"code": code})
    value = json.loads(result["output"])
    gas = "GASToolsets.AbilitySystemInspectorToolset"
    c.tool("describe_toolset", {"toolset_name": gas})
    value["attributes"] = c.call(gas, "GetAttributeValues", {"actor": {"refPath": value["boss"]}})
    args.output.write_text(json.dumps(value, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(value))


if __name__ == "__main__":
    main()
