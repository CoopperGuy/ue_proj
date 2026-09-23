// Execute in Codex functions.exec, outside Unreal's ProgrammaticToolset.
// Every awaited call is a separate native MCP request; no enclosing editor transaction.
const desiredSkills = {
  "LavaEruption": {
    "skillId": 1013,
    "skillName": "용암 분출",
    "description": "지정 지점에 4초 동안 용암 지대를 만듭니다. 반경 240cm 안의 적에게 0.5초마다 기본 피해 28을 줍니다.",
    "icon": "/Game/Skills/LavaEruption/Icons/T_LavaEruption.T_LavaEruption",
    "abilityClass": "/Script/CoreUObject.Class'/Script/ARPG.DiaGameplayAbility'",
    "extraData": {},
    "steps": [
      {
        "_structType": "/Script/ARPG.GASGroundSpawnStepData",
        "GroundData": {
          "bUseOwnerRotation": false,
          "Radius": 240,
          "SkillActorClass": "/Script/Engine.BlueprintGeneratedClass'/Game/Skills/LavaEruption/BP_LavaEruption.BP_LavaEruption_C'",
          "LifeSpan": 4
        },
        "bUseLastStepEndLocation": false,
        "StartDelay": 0,
        "bWaitUntilFinished": true
      }
    ],
    "skillType": "Magic",
    "targetType": "Ground",
    "abilityTags": {
      "GameplayTags": [
        {
          "TagName": "SkillType.Character.Player"
        }
      ],
      "ParentTags": [
        {
          "TagName": "SkillType.Character"
        },
        {
          "TagName": "SkillType"
        }
      ]
    },
    "cancelAbilitiesWithTag": {
      "GameplayTags": [],
      "ParentTags": []
    },
    "blockAbilitiesWithTag": {
      "GameplayTags": [],
      "ParentTags": []
    },
    "manaCostTags": {
      "TagName": "GASData.ManaCost"
    },
    "coolDownTags": {
      "TagName": "GASData.CoolDown.1013"
    },
    "baseDamage": 28,
    "manaCost": 24,
    "cooldownDuration": 7,
    "castTime": 0.2,
    "hitCount": 8,
    "hitInterval": 0.5,
    "damageMultiplierPerHit": 1,
    "castAnimation": "/Game/Blueprint/Character/Animation/AM_NormalAttack.AM_NormalAttack",
    "castEffect": "None",
    "legacyCastEffect": "None",
    "castSound": "None",
    "maxLevel": 5,
    "requiredCharacterLevel": 3,
    "prerequisiteSkills": [],
    "effectsToApplyOnHit": [],
    "effectsToApplyOnSelf": [],
    "variantIds": [
      24,
      25,
      26
    ]
  },
  "ArcScatter": {
    "skillId": 1014,
    "skillName": "전격 산탄",
    "description": "전방으로 번개 탄환 3개를 부채꼴로 발사합니다. 탄환마다 기본 피해 36을 줍니다.",
    "icon": "/Game/Skills/ArcScatter/Icons/T_ArcScatter.T_ArcScatter",
    "abilityClass": "/Script/CoreUObject.Class'/Script/ARPG.DiaGameplayAbility'",
    "extraData": {},
    "steps": [
      {
        "_structType": "/Script/ARPG.GASProjectileSpawnStepData",
        "ProjectileData": {
          "ProjectileOffset": {
            "X": 120,
            "Y": 0,
            "Z": 0
          },
          "MinimumRange": 120,
          "bUseOwnerRotation": false,
          "bFireMultipleProjectiles": true,
          "ProjectileCount": 3,
          "SpreadAngle": 12,
          "SkillActorClass": "/Script/Engine.BlueprintGeneratedClass'/Game/Skills/ArcScatter/BP_ArcScatter.BP_ArcScatter_C'",
          "LifeSpan": 1.2
        },
        "bUseLastStepEndLocation": true,
        "StartDelay": 0,
        "bWaitUntilFinished": true
      }
    ],
    "skillType": "Magic",
    "targetType": "Enemy",
    "abilityTags": {
      "GameplayTags": [
        {
          "TagName": "SkillType.Character.Player"
        }
      ],
      "ParentTags": [
        {
          "TagName": "SkillType.Character"
        },
        {
          "TagName": "SkillType"
        }
      ]
    },
    "cancelAbilitiesWithTag": {
      "GameplayTags": [],
      "ParentTags": []
    },
    "blockAbilitiesWithTag": {
      "GameplayTags": [],
      "ParentTags": []
    },
    "manaCostTags": {
      "TagName": "GASData.ManaCost"
    },
    "coolDownTags": {
      "TagName": "GASData.CoolDown.1014"
    },
    "baseDamage": 36,
    "manaCost": 16,
    "cooldownDuration": 3.5,
    "castTime": 0.12,
    "hitCount": 1,
    "hitInterval": 0,
    "damageMultiplierPerHit": 1,
    "castAnimation": "/Game/Blueprint/Character/Animation/AM_NormalAttack.AM_NormalAttack",
    "castEffect": "None",
    "legacyCastEffect": "None",
    "castSound": "None",
    "maxLevel": 5,
    "requiredCharacterLevel": 2,
    "prerequisiteSkills": [],
    "effectsToApplyOnHit": [],
    "effectsToApplyOnSelf": [],
    "variantIds": [
      27,
      28,
      29
    ]
  },
  "FrostRush": {
    "skillId": 1015,
    "skillName": "서리 돌진",
    "description": "바라보는 방향으로 최대 600cm 돌진한 뒤 도착 지점에 1.8초 동안 서리 지대를 남깁니다. 반경 220cm 안의 적에게 0.6초마다 기본 피해 55와 둔화를 적용합니다.",
    "icon": "/Game/Skills/FrostRush/Icons/T_FrostRush.T_FrostRush",
    "abilityClass": "/Script/CoreUObject.Class'/Script/ARPG.DiaGameplayAbility'",
    "extraData": {},
    "steps": [
      {
        "_structType": "/Script/ARPG.GASChargeStepData",
        "ChargeData": {
          "ChargeDelay": 0,
          "ChargeDuration": 0.25,
          "UpdateTime": 0.05,
          "ChargeDist": 600,
          "PathOffsetCurve": "None"
        },
        "StartDelay": 0,
        "bWaitUntilFinished": true
      },
      {
        "_structType": "/Script/ARPG.GASGroundSpawnStepData",
        "GroundData": {
          "bUseOwnerRotation": false,
          "Radius": 220,
          "SkillActorClass": "/Script/Engine.BlueprintGeneratedClass'/Game/Skills/FrostRush/BP_FrostRush.BP_FrostRush_C'",
          "LifeSpan": 1.8
        },
        "bUseLastStepEndLocation": true,
        "StartDelay": 0,
        "bWaitUntilFinished": true
      }
    ],
    "skillType": "Magic",
    "targetType": "Self",
    "abilityTags": {
      "GameplayTags": [
        {
          "TagName": "SkillType.Character.Player"
        }
      ],
      "ParentTags": [
        {
          "TagName": "SkillType.Character"
        },
        {
          "TagName": "SkillType"
        }
      ]
    },
    "cancelAbilitiesWithTag": {
      "GameplayTags": [],
      "ParentTags": []
    },
    "blockAbilitiesWithTag": {
      "GameplayTags": [],
      "ParentTags": []
    },
    "manaCostTags": {
      "TagName": "GASData.ManaCost"
    },
    "coolDownTags": {
      "TagName": "GASData.CoolDown.1015"
    },
    "baseDamage": 55,
    "manaCost": 20,
    "cooldownDuration": 6,
    "castTime": 0.05,
    "hitCount": 3,
    "hitInterval": 0.6,
    "damageMultiplierPerHit": 1,
    "castAnimation": "/Game/Blueprint/Character/Animation/AM_NormalAttack.AM_NormalAttack",
    "castEffect": "None",
    "legacyCastEffect": "None",
    "castSound": "None",
    "maxLevel": 5,
    "requiredCharacterLevel": 3,
    "prerequisiteSkills": [],
    "effectsToApplyOnHit": [
      "/Script/Engine.BlueprintGeneratedClass'/Game/Skills/FrostRush/GE_FrostRush_Slow.GE_FrostRush_Slow_C'"
    ],
    "effectsToApplyOnSelf": [],
    "variantIds": [
      30,
      31,
      32
    ]
  }
};
const desiredVariants = {
  "LavaEruption_Area": {
    "variantId": 24,
    "variantName": "넘치는 용암",
    "description": "용암 지대의 반경이 35% 증가합니다.",
    "icon": "/Game/Skills/LavaEruption/Icons/T_LavaEruption_Area.T_LavaEruption_Area",
    "modifierValue": 1.35,
    "variantTag": {
      "TagName": "GASData.Variant.AreaMultiplier"
    },
    "variantExtraData": {}
  },
  "LavaEruption_Cooldown": {
    "variantId": 25,
    "variantName": "재분출",
    "description": "용암 분출의 재사용 대기시간이 25% 감소합니다.",
    "icon": "/Game/Skills/LavaEruption/Icons/T_LavaEruption_Cooldown.T_LavaEruption_Cooldown",
    "modifierValue": 0.75,
    "variantTag": {
      "TagName": "GASData.Variant.CooldownReduction"
    },
    "variantExtraData": {}
  },
  "LavaEruption_Mana": {
    "variantId": 26,
    "variantName": "안정된 화맥",
    "description": "용암 분출의 마나 소모량이 30% 감소합니다.",
    "icon": "/Game/Skills/LavaEruption/Icons/T_LavaEruption_Mana.T_LavaEruption_Mana",
    "modifierValue": 0.7,
    "variantTag": {
      "TagName": "GASData.Variant.ManaCostMultiplier"
    },
    "variantExtraData": {}
  },
  "ArcScatter_Multiple": {
    "variantId": 27,
    "variantName": "다중 방전",
    "description": "번개 탄환이 2개 늘어나 한 번에 총 5개를 발사합니다.",
    "icon": "/Game/Skills/ArcScatter/Icons/T_ArcScatter_Multiple.T_ArcScatter_Multiple",
    "modifierValue": 2,
    "variantTag": {
      "TagName": "GASData.Variant.MultipleSkill"
    },
    "variantExtraData": {}
  },
  "ArcScatter_Pierce": {
    "variantId": 28,
    "variantName": "관통 전류",
    "description": "각 번개 탄환이 최대 3명의 적을 관통합니다.",
    "icon": "/Game/Skills/ArcScatter/Icons/T_ArcScatter_Pierce.T_ArcScatter_Pierce",
    "modifierValue": 3,
    "variantTag": {
      "TagName": "GASData.Variant.Pierce"
    },
    "variantExtraData": {}
  },
  "ArcScatter_Fork": {
    "variantId": 29,
    "variantName": "가지 번개",
    "description": "본체 탄환이 적중할 때 번개 탄환 2개로 분열합니다. 분열체는 다시 분열하지 않습니다.",
    "icon": "/Game/Skills/ArcScatter/Icons/T_ArcScatter_Fork.T_ArcScatter_Fork",
    "modifierValue": 2,
    "variantTag": {
      "TagName": "GASData.Variant.Fork"
    },
    "variantExtraData": {}
  },
  "FrostRush_Area": {
    "variantId": 30,
    "variantName": "넓은 서릿발",
    "description": "도착 지점에 생성되는 서리 지대의 반경이 40% 증가합니다.",
    "icon": "/Game/Skills/FrostRush/Icons/T_FrostRush_Area.T_FrostRush_Area",
    "modifierValue": 1.4,
    "variantTag": {
      "TagName": "GASData.Variant.AreaMultiplier"
    },
    "variantExtraData": {}
  },
  "FrostRush_Freeze": {
    "variantId": 31,
    "variantName": "동결 지대",
    "description": "서리 지대에 적중한 적에게 1.2초 빙결을 적용합니다.",
    "icon": "/Game/Skills/FrostRush/Icons/T_FrostRush_Freeze.T_FrostRush_Freeze",
    "modifierValue": 1.2,
    "variantTag": {
      "TagName": "GASData.Variant.FreezeStack"
    },
    "variantExtraData": {
      "_structType": "/Script/ARPG.SkillVariantGameplayEffectData",
      "TargetEffects": [
        {
          "EffectClass": "/Script/Engine.BlueprintGeneratedClass'/Game/Skills/FrostRush/GE_FrostRush_Freeze.GE_FrostRush_Freeze_C'",
          "SetByCallerMagnitudes": {}
        }
      ]
    }
  },
  "FrostRush_Cooldown": {
    "variantId": 32,
    "variantName": "겨울 걸음",
    "description": "서리 돌진의 재사용 대기시간이 25% 감소합니다.",
    "icon": "/Game/Skills/FrostRush/Icons/T_FrostRush_Cooldown.T_FrostRush_Cooldown",
    "modifierValue": 0.75,
    "variantTag": {
      "TagName": "GASData.Variant.CooldownReduction"
    },
    "variantExtraData": {}
  }
};
const dt = "editor_toolset.toolsets.data_table.DataTableTools";
const at = "editor_toolset.toolsets.asset.AssetTools";
const ref = name => ({refPath: "/Game/Datatable/"+name+"."+name});
async function call(group, name, args) {
  const result = await tools.mcp__UnrealEngine__call_tool({toolset_name:group,tool_name:name,arguments:args});
  if(result.isError) throw new Error(JSON.stringify(result));
  const body = result.content.find(c=>c.type==="text");
  return JSON.parse(body.text).returnValue;
}
async function read(name) {
  const data_table=ref(name);
  const row_names=await call(dt,"list_rows",{data_table});
  return JSON.parse(await call(dt,"get_rows",{data_table,row_names}));
}
function normalize(value) {
  if(Array.isArray(value)){value.forEach(normalize);return;}
  if(!value || typeof value!=="object")return;
  for(const [key,child] of Object.entries(value)){
    if(key==="SetByCallerMagnitudes" && child){
      for(const [tag,number] of Object.entries(child)){
        if(number && typeof number==="object" && Object.keys(number).length===1 && "SetByCallerMagnitudes" in number) child[tag]=number.SetByCallerMagnitudes;
      }
    } else normalize(child);
  }
}
const beforeSkills=await read("DT_GASSkillData");
const beforeVariants=await read("DT_SkillVariantion");
const beforeJobs=await read("DT_JobSkillSet");
store("elementRegistrationBefore",{skills:beforeSkills,variants:beforeVariants,jobs:beforeJobs});
for(const [old,desired,field] of [[beforeSkills,desiredSkills,"skillId"],[beforeVariants,desiredVariants,"variantId"]]){
  const occupied=new Set(Object.entries(old).filter(([key])=>!(key in desired)).map(([,row])=>row[field]));
  for(const [key,row] of Object.entries(desired)){
    if(occupied.has(row[field]) || (old[key] && ![0,row[field]].includes(old[key][field]))) throw new Error("ID or row conflict: "+key);
  }
}
for(const [name,old,desired] of [["DT_GASSkillData",beforeSkills,desiredSkills],["DT_SkillVariantion",beforeVariants,desiredVariants]]){
  const patch=JSON.parse(JSON.stringify(desired));
  if(name==="DT_SkillVariantion"){
    for(const [key,row] of Object.entries(old)){
      if(!(key in patch) && row.variantExtraData && Object.keys(row.variantExtraData).length){
        patch[key]={variantExtraData:JSON.parse(JSON.stringify(row.variantExtraData))};
        normalize(patch[key]);
      }
    }
  }
  const missing=Object.keys(desired).filter(key=>!(key in old));
  if(missing.length) await call(dt,"add_rows",{data_table:ref(name),row_names:missing});
  await call(dt,"set_rows",{data_table:ref(name),values:JSON.stringify(patch)});
  if(!await call(at,"save_assets",{asset_paths:["/Game/Datatable/"+name]})) throw new Error("Save failed: "+name);
}
const skillIds=[...beforeJobs.WarriorSkillSet.skillIds];
for(const row of Object.values(desiredSkills))if(!skillIds.includes(row.skillId))skillIds.push(row.skillId);
await call(dt,"set_rows",{data_table:ref("DT_JobSkillSet"),values:JSON.stringify({WarriorSkillSet:{skillIds}})});
if(!await call(at,"save_assets",{asset_paths:["/Game/Datatable/DT_JobSkillSet"]}))throw new Error("Job table save failed");
text({registeredSkills:Object.keys(desiredSkills),registeredVariants:Object.keys(desiredVariants),warriorSkillIds:skillIds});
