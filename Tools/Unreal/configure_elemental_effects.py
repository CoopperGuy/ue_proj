"""Run this file through Unreal native ProgrammaticToolset.execute_tool_script."""
import json
def call(s,n,a):
    return execute_tool(s+"."+n,json.dumps(a)).get("returnValue")
def ref(system,script="",module="",inp="",renderer=-1):
    return {"system":system,"emitterName":"Fountain","scriptName":script,"moduleName":module,"rendererIndex":renderer,"inputNameStack":[inp] if inp else []}
def set_input(system,script,module,inp,value,kind="Niagara.NiagaraFloat"):
    if kind=="Niagara.NiagaraFloat":
        value={"value":value}
    return call("NiagaraToolsets.NiagaraToolset_System","SetStackInputData",{"stackInputRef":ref(system,script,module,inp),"inputData":{"struct":{"refPath":"/Script/"+kind},"value":value}})
def run():
    results=[]
    for name in ["LavaEruption","ArcScatter","FrostRush"]:
        root="/Game/Skills/"+name
        ground=name!="ArcScatter"
        for kind in ["Trail","Impact"]:
            template="/Game/Skills/FrostLance/Effects/NS_FrostLance_"+kind
            system=call("NiagaraToolsets.NiagaraToolset_System","CreateNiagaraSystem",{"assetName":"NS_"+name+"_"+kind,"assetPath":root+"/Effects","templateSystem":{"refPath":template+".NS_FrostLance_"+kind}})
            data=call("NiagaraToolsets.NiagaraToolset_System","GetRendererData",{"rendererRef":ref(system,renderer=0)})
            meshes=json.loads(data["propertyValues"])["Meshes"]
            meshes[0]["mesh"]={"refPath":root+"/Meshes/SM_"+name+".SM_"+name}
            call("NiagaraToolsets.NiagaraToolset_System","SetRendererData",{"renderer":ref(system,renderer=0),"rendererData":{"propertyValues":json.dumps({"Meshes":meshes,"bCastShadows":False})}})
            call("NiagaraToolsets.NiagaraToolset_System","SetEmitterData",{"emitter":ref(system),"emitterData":{"propertyValues":json.dumps({"bLocalSpace":ground and kind=="Trail"})}})
            set_input(system,"EmitterUpdateScript","SpawnRate","SpawnRate",24 if ground and kind=="Trail" else (45 if kind=="Trail" else 120))
            for inp,v in [("Lifetime Min",.18 if kind=="Impact" else .2),("Lifetime Max",.45 if kind=="Impact" else .5)]:
                set_input(system,"ParticleSpawnScript","InitializeParticle",inp,v)
            scale=({"x":.025,"y":.025,"z":.09} if ground else {"x":.1,"y":.15,"z":.2})
            set_input(system,"ParticleSpawnScript","InitializeParticle","Mesh Scale",scale,"CoreUObject.Vector3f")
            set_input(system,"ParticleSpawnScript","ShapeLocation","Sphere Radius",(175 if ground and kind=="Trail" else 24))
            if ground and kind=="Trail":
                set_input(system,"ParticleSpawnScript","ShapeLocation","Non Uniform Scale",{"x":1,"y":1,"z":.08},"CoreUObject.Vector3f")
            set_input(system,"ParticleSpawnScript","AddVelocity","Velocity",{"x":0,"y":0,"z":60 if ground else 0},"CoreUObject.Vector3f")
            set_input(system,"ParticleUpdateScript","GravityForce","Gravity",{"x":0,"y":0,"z":-90 if kind=="Impact" else 0},"CoreUObject.Vector3f")
            results.append(system["refPath"])
        bp={"refPath":root+"/BP_"+name+".BP_"+name}
        values={"skillEffect":{"refPath":root+"/Effects/NS_"+name+"_Trail.NS_"+name+"_Trail"},
                "hitEffect":{"refPath":root+"/Effects/NS_"+name+"_Impact.NS_"+name+"_Impact"}}
        if ground:
            values["radius"]=240 if name=="LavaEruption" else 220
        else:
            values["projectileSpeed"]=2300
        call("editor_toolset.toolsets.object.ObjectTools","set_properties",{"instance":bp,"values":json.dumps(values)})
        if ground:
            comp=root+"/BP_"+name+".Default__BP_"+name+"_C:"
            call("editor_toolset.toolsets.object.ObjectTools","set_properties",{"instance":{"refPath":comp+"SphereComp"},"values":json.dumps({"bGenerateOverlapEvents":False,"sphereRadius":values["radius"]})})
            if name=="FrostRush":
                for suffix in ["SkillObjectMesh","SkillEffectComp"]:
                    call("editor_toolset.toolsets.object.ObjectTools","set_properties",{"instance":{"refPath":comp+suffix},"values":json.dumps({"relativeLocation":{"x":0,"y":0,"z":-88}})})
        call("editor_toolset.toolsets.blueprint.BlueprintTools","compile_blueprint",{"blueprint":bp,"warnings_as_errors":True})
    if not call("editor_toolset.toolsets.asset.AssetTools","exists",{"path":"/Game/Skills/FrostRush/GE_FrostRush_Freeze"}):
        call("editor_toolset.toolsets.asset.AssetTools","duplicate",{"path":"/Game/Skills/FrostLance/GE_FrostLance_Freeze","new_path":"/Game/Skills/FrostRush/GE_FrostRush_Freeze"})
    call("editor_toolset.toolsets.blueprint.BlueprintTools","compile_blueprint",{"blueprint":{"refPath":"/Game/Skills/FrostRush/GE_FrostRush_Freeze.GE_FrostRush_Freeze"},"warnings_as_errors":True})
    slow="/Game/Skills/FrostRush/GE_FrostRush_Slow"
    if not call("editor_toolset.toolsets.asset.AssetTools","exists",{"path":slow}):
        call("editor_toolset.toolsets.asset.AssetTools","duplicate",{"path":"/Game/Blueprint/GAS_Skill/GameplayEffect/GE_IceSlow2s_half","new_path":slow})
    slow_ref={"refPath":slow+".GE_FrostRush_Slow"}
    duration=json.loads(call("editor_toolset.toolsets.object.ObjectTools","get_properties",{"instance":slow_ref,"properties":["durationMagnitude"]}))
    duration["durationMagnitude"]["magnitudeCalculationType"]="ScalableFloat"
    duration["durationMagnitude"]["scalableFloatMagnitude"]["value"]=2
    call("editor_toolset.toolsets.object.ObjectTools","set_properties",{"instance":slow_ref,"values":json.dumps(duration)})
    call("editor_toolset.toolsets.blueprint.BlueprintTools","compile_blueprint",{"blueprint":slow_ref,"warnings_as_errors":True})
    return {"effects":results}
