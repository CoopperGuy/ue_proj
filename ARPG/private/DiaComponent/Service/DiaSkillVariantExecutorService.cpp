// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Service/DiaSkillVariantExecutorService.h"
#include "DiaComponent/Skill/Executor/DiaSkillVariantSpawnExecutor.h"
#include "DiaComponent/Skill/Executor/DiaSkillHitVariantExecutor.h"
#include "DiaComponent/Skill/Executor/DiaSkillVariantExecutor_ActModi.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"
#include "Skill/DiaSkillActor.h"
#include "Logging/ARPGLogChannels.h"

void UDiaSkillVariantExecutorService::InitializeExecutorService()
{
	if (!SpawnExecutor)
	{
		SpawnExecutor = NewObject<UDiaSkillVariantSpawnExecutor>(this);
		SpawnExecutor->InitializeExecutor();
	}

	if (!HitExecutor)
	{
		HitExecutor = NewObject<UDiaSkillHitVariantExecutor>(this);
		HitExecutor->InitializeExecutor();
	}

	if(!ActModiExecutor)
	{
		ActModiExecutor = NewObject<UDiaSkillVariantExecutor_ActModi>(this);
		ActModiExecutor->InitializeExecutor();
	}
}

void UDiaSkillVariantExecutorService::ExecuteActiveModifierVariants(const TArray<UDiaSkillVariant*>& Variants,
	FDiaSkillVariantContext& Context,
	const UDiaGameplayAbility* Ability,
	FSkillModifierRuntime& OutRuntime)
{
	InitializeExecutorService();
	ActModiExecutor->ExecuteEffect(Variants, Context, Ability, OutRuntime);
}

void UDiaSkillVariantExecutorService::ExecuteSpawnVariants(
	const TSet<int32>& VariantIDs,
	const TMap<int32, UDiaSkillVariant*>& VariantCache,
	FDiaSkillVariantContext& Context,
	UDiaGameplayAbility* Ability,
	FDiaSkillSpawnFinishedDelegate OnFinished)
{
	if (!Context.SkillActorClass)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillVariantExecutorService::ExecuteSpawnVariants: SkillActorClass가 유효하지 않습니다."));
		OnFinished.ExecuteIfBound();
		return;
	}

	// VariantID 배열을 UDiaSkillVariant* 배열로 변환
	TArray<UDiaSkillVariant*> VariantsToApply;
	VariantsToApply.Reserve(VariantIDs.Num());

	for (const int32 VariantID : VariantIDs)
	{
		if (UDiaSkillVariant* const* FoundVariant = VariantCache.Find(VariantID))
		{
			VariantsToApply.Add(*FoundVariant);
		}
	}

	InitializeExecutorService();
	SpawnExecutor->ExecuteEffect(VariantsToApply, Context, Ability, OnFinished);
}

void UDiaSkillVariantExecutorService::ExecuteHitVariants(
	const TArray<UDiaSkillVariant*>& Variants,
	FDiaSkillVariantContext& Context,
	UDiaGameplayAbility* Ability)
{
	InitializeExecutorService();
	HitExecutor->ExecuteEffect(Variants, Context, Ability);
}

void UDiaSkillVariantExecutorService::ExecuteHitVariants(
	const TArray<UDiaSkillVariant*>& Variants,
	FDiaSkillVariantContext& Context,
	const UDiaGameplayAbility* Ability,
	FSkillHitRuntime& OutRuntime)
{
	InitializeExecutorService();
	HitExecutor->ExecuteEffect(Variants, Context, Ability, OutRuntime);
}
