"""Run this file through Unreal native ProgrammaticToolset.execute_tool_script."""
import json
def call(toolset, name, args):
    return execute_tool(toolset + "." + name, json.dumps(args)).get("returnValue")
def run():
    created=[]
    variants={"LavaEruption":["Area","Cooldown","Mana"],"ArcScatter":["Multiple","Pierce","Fork"],"FrostRush":["Area","Freeze","Cooldown"]}
    for name in variants:
        root="/Game/Skills/"+name
        bp={"refPath":root+"/BP_"+name+".BP_"+name}
        if not call("editor_toolset.toolsets.asset.AssetTools", "exists", {"path":bp["refPath"]}):
            bp=call("editor_toolset.toolsets.blueprint.BlueprintTools","create",{"folder_path":root,"asset_name":"BP_"+name,"asset_type":{"refPath":"/Script/ARPG."+("DiaProjectile" if name=="ArcScatter" else "DiaGroundObj")}})
        mesh=[{"refPath":root+"/Meshes/SM_"+name+".SM_"+name}]
        if not call("editor_toolset.toolsets.asset.AssetTools", "exists", {"path":mesh[0]["refPath"]}):
            mesh=call("editor_toolset.toolsets.static_mesh.StaticMeshTools","import_file",{"folder_path":root+"/Meshes","asset_name":"SM_"+name,"source_file":"C:/Users/crise/Documents/Unreal Projects/ARPG/ArtSource/ElementalPack/"+name+"/SM_"+name+".fbx","import_materials":False,"import_textures":False,"combine_meshes":True})
        icons=[]
        for suffix in [""]+["_"+x for x in variants[name]]:
            icon="T_"+name+suffix
            path=root+"/Icons/"+icon+"."+icon
            if call("editor_toolset.toolsets.asset.AssetTools", "exists", {"path":path}):
                icons.append([{"refPath":path}])
            else:
                icons.append(call("editor_toolset.toolsets.texture.TextureTools","import_file",{"folder_path":root+"/Icons","asset_name":icon,"source_file":"C:/Users/crise/Documents/Unreal Projects/ARPG/ArtSource/ElementalPack/"+name+"/"+icon+".png"}))
        created.append({"name":name,"bp":bp,"mesh":mesh,"icons":icons})
    return {"created":created}
