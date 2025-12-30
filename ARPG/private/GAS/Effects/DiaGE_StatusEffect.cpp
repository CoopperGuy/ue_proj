// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Effects/DiaGE_StatusEffect.h"
#include "GameplayTagsManager.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GAS/DiaGameplayTags.h"

UDiaGE_StatusEffect::UDiaGE_StatusEffect()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;
    DefaultDuration = 2.0f;
    DurationMagnitude = FScalableFloat(DefaultDuration);

    bAllowStacking = true;
    MaxStackCount = 5;
    StackLimitCount = 5;
    StackingType = EGameplayEffectStackingType::AggregateByTarget; // 필요 시 Source로 변경
    StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;
    StackPeriodResetPolicy = EGameplayEffectStackingPeriodPolicy::ResetOnSuccessfulApplication;

    UTargetTagsGameplayEffectComponent* TargetTagComp =
        CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TargetTags"));

    FInheritedTagContainer TagContainer;
    TagContainer.Added.AddTag(FDiaGameplayTags::Get().State_Stunned);
    TargetTagComp->SetAndApplyTargetTagChanges(TagContainer);

    GEComponents.Add(TargetTagComp);
}





