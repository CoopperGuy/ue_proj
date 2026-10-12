#include "GAS/Effects/DiaGE_EquipmentSlow.h"
#include "GAS/Effects/DiaGameplayEffectCompat.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Types/DiaEquipmentOptionRules.h"

UDiaGE_EquipmentSlow::UDiaGE_EquipmentSlow()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DurationMagnitude = FScalableFloat(DiaEquipmentOptionRules::FrostDuration);
    DiaGameplayEffectCompat::SetStackingType(this, EGameplayEffectStackingType::AggregateByTarget);
    StackLimitCount = 1;
    StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;

    FSetByCallerFloat Caller;
    Caller.DataTag = FDiaGameplayTags::Get().GASData_StatusValue;
    FGameplayModifierInfo Modifier;
    Modifier.Attribute = UDiaAttributeSet::GetMovementSpeedAttribute();
    Modifier.ModifierOp = EGameplayModOp::Multiplicitive;
    Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(Caller);
    Modifiers.Add(Modifier);

    UTargetTagsGameplayEffectComponent* Tags = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("SlowTags"));
    FInheritedTagContainer Container;
    Container.Added.AddTag(FDiaGameplayTags::Get().State_Slowed);
    Tags->SetAndApplyTargetTagChanges(Container);
    GEComponents.Add(Tags);
}
