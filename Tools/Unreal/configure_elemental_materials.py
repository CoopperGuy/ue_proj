"""Run this file through Unreal native ProgrammaticToolset.execute_tool_script."""
import json
def call(group, name, args):
    return execute_tool("editor_toolset.toolsets."+group+"."+name, json.dumps(args)).get("returnValue")
def run():
    palettes={"LavaEruption":[[.055,.009,.004],[.9,.07,.005],[2,.6,.025]],
              "ArcScatter":[[.025,.02,.2],[.22,.13,.95],[.35,1.7,2]],
              "FrostRush":[[.015,.14,.22],[.025,.58,.7],[.5,1.6,1.3]]}
    result=[]
    for name, colors in palettes.items():
        root="/Game/Skills/"+name
        mesh={"refPath":root+"/Meshes/SM_"+name+".SM_"+name}
        for slot, rgb in zip(["Facet","Edge","Core"], colors):
            mat=call("material.MaterialTools","create_material",{"folder_path":root+"/Materials","asset_name":"M_"+name+"_"+slot})
            call("object.ObjectTools","set_properties",{"instance":mat,"values":json.dumps({"bUsedWithNiagaraMeshParticles":True})})
            expr=call("material.MaterialTools","add_expression",{"material_or_function":mat,"expression_class":{"refPath":"/Script/Engine.MaterialExpressionConstant3Vector"},"x":-300,"y":0})
            call("object.ObjectTools","set_properties",{"instance":expr,"values":json.dumps({"constant":dict(zip(["r","g","b","a"],rgb+[1]))})})
            call("material.MaterialTools","connect_to_output",{"expression":expr,"output_name":"","material_property":"MP_BaseColor"})
            if slot=="Core":
                call("material.MaterialTools","connect_to_output",{"expression":expr,"output_name":"","material_property":"MP_EmissiveColor"})
            call("material.MaterialTools","recompile",{"material_or_function":mat})
            call("static_mesh.StaticMeshTools","set_material",{"mesh":mesh,"slot_name":slot,"material":mat})
        bp={"refPath":root+"/BP_"+name+".BP_"+name}
        call("object.ObjectTools","set_properties",{"instance":bp,"values":json.dumps({"lifeSpan":{"LavaEruption":4,"ArcScatter":1.2,"FrostRush":1.8}[name]})})
        comp={"refPath":root+"/BP_"+name+".Default__BP_"+name+"_C:SkillObjectMesh"}
        call("object.ObjectTools","set_properties",{"instance":comp,"values":json.dumps({"staticMesh":mesh,"relativeScale3D":{"x":1,"y":1,"z":1},"castShadow":False})})
        call("blueprint.BlueprintTools","compile_blueprint",{"blueprint":bp,"warnings_as_errors":True})
        result.append(name)
    return {"materials_and_meshes":result}
