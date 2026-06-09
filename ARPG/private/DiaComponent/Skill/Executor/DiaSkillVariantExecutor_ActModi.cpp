// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Executor/DiaSkillVariantExecutor_ActModi.h"
#include "DiaComponent/Skill/Effect/DiaVariantEffect_ActiveModifier.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"
#include "Logging/ARPGLogChannels.h"


void UDiaSkillVariantExecutor_ActModi::InitializeExecutor()
{
	UE_LOG(LogARPG, Log, TEXT("UDiaSkillHitVariantExecutor::InitializeExecutor - Initializing Hit Variant Executor"));
	RegisteredEffects.Add(NewObject<UDiaVariantEffect_ActiveModifier>(this));
}

void UDiaSkillVariantExecutor_ActModi::ExecuteEffect(const TArray<class UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, const UDiaGameplayAbility* Ability, FSkillVariantRuntime& OutRuntime)
{
	FSkillModifierRuntime& ModifierRuntime = static_cast<FSkillModifierRuntime&>(OutRuntime);
	ApplyEffects(Variants, Context, ModifierRuntime);
}
