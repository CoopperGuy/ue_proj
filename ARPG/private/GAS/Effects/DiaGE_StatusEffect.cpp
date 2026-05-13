// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Effects/DiaGE_StatusEffect.h"
#include "GAS/Effects/DiaGameplayEffectCompat.h"
#include "GameplayTagsManager.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "GAS/DiaGameplayTags.h"

UDiaGE_StatusEffect::UDiaGE_StatusEffect()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;

    FSetByCallerFloat DurationCaller;
    DurationCaller.DataTag = FDiaGameplayTags::Get().GASData_Duration;
    DurationMagnitude = FGameplayEffectModifierMagnitude(DurationCaller);

    DiaGameplayEffectCompat::SetStackingType(this, EGameplayEffectStackingType::AggregateByTarget);
    StackLimitCount = 1;
    StackDurationRefreshPolicy =
        EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;
    StackPeriodResetPolicy =
        EGameplayEffectStackingPeriodPolicy::ResetOnSuccessfulApplication;

    Modifiers.Reset();
}



