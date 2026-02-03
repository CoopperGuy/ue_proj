// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Service/DiaSkillVariantExecutorService.h"
#include "DiaComponent/Skill/Executor/DiaSkillVariantSpawnExecutor.h"
#include "DiaComponent/Skill/Executor/DiaSkillHitVariantExecutor.h"
#include "DiaComponent/Skill/Effect/DiaVariantEffect_MultipleShot.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"
#include "Skill/DiaSkillActor.h"

void UDiaSkillVariantExecutorService::InitializeExecutorService()
{
}

void UDiaSkillVariantExecutorService::ExecuteVariants(
	const TArray<int32>& VariantIDs,
	const TMap<int32, UDiaSkillVariant*>& VariantCache,
	FDiaSkillVariantContext& Context,
	UDiaGameplayAbility* Ability)
{
	if (!Context.SkillActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiaSkillVariantExecutorService::ExecuteVariants: SkillActorClass가 유효하지 않습니다."));
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

	// Context의 SkillActor 타입에 따라 적절한 Executor 선택
	// 현재는 SpawnExecutor만 사용 (추후 확장 가능)
	UDiaSkillVariantSpawnExecutor* SpawnExecutor = NewObject<UDiaSkillVariantSpawnExecutor>(this);
	SpawnExecutor->ExecuteEffect(VariantsToApply, Context, Ability);
}