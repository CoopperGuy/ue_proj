// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Executor/DiaSkillHitVariantExecutor.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect_Pierce.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"
#include "Skill/DiaSkillActor.h"

void UDiaSkillHitVariantExecutor::InitializeExecutor()
{
	EffectsByTag.Add(FDiaGameplayTags::Get().GASData_Pierce, NewObject<UDiaSkillVariantEffect_Pierce>(this));
}

void UDiaSkillHitVariantExecutor::ExecuteEffect(const TArray<class UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, const UDiaGameplayAbility* Ability)
{
	FSkillHitRuntime Runtime;
	ApplyEffects(Variants, Context, Runtime);
}

void UDiaSkillHitVariantExecutor::ExecuteEffect(const TArray<class UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, const UDiaGameplayAbility* Ability, FSkillHitRuntime& OutRuntime)
{
	// 기존 PierceCount를 Runtime에 설정
	OutRuntime.PierceCount = Context.SkillActor ? Context.SkillActor->GetPierceCount() : 0;
	ApplyEffects(Variants, Context, OutRuntime);
}
