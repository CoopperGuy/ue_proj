#include "GAS/Effects/DiaGameplayEffect_Damage.h"
#include "GAS/DiaAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

#include "GAS/Effects/Exec_Damage.h"

UDiaGameplayEffect_Damage::UDiaGameplayEffect_Damage()
{
    // Instant effect (한번만 적용)
    DurationPolicy = EGameplayEffectDurationType::Instant;

    // Execution 계산 사용(Attack/Defense 캡처 및 SetByCaller 사용)
    FGameplayEffectExecutionDefinition ExecDef;
    ExecDef.CalculationClass = UExec_Damage::StaticClass();
    Executions.Add(ExecDef);

    UAssetTagsGameplayEffectComponent* AssetTagComp = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>(TEXT("AssetTags"));
    FInheritedTagContainer TagContainer;
    TagContainer.Added.AddTag(FGameplayTag::RequestGameplayTag(FName("GASData.DamageBase")));
    TagContainer.Added.AddTag(FGameplayTag::RequestGameplayTag(FName("GASData.CritMultiplier")));
    AssetTagComp->SetAndApplyAssetTagChanges(TagContainer);
    GEComponents.Add(AssetTagComp);
}