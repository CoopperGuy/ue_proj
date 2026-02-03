// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/DiaSkillVariantExecutor.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaSkillHitVariantExecutor.generated.h"

class UDiaSkillVariantEffect;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaSkillHitVariantExecutor : public UDiaSkillVariantExecutor
{
	GENERATED_BODY()
public:
	virtual void InitializeExecutor();

	void ExecuteEffect(
		const TArray<UDiaSkillVariant*>& Variants,
		FDiaSkillVariantContext& Context,
		const UDiaGameplayAbility* Ability);

	// Runtime을 반환하는 버전 (PierceCount 업데이트를 위해)
	void ExecuteEffect(
		const TArray<UDiaSkillVariant*>& Variants,
		FDiaSkillVariantContext& Context,
		const UDiaGameplayAbility* Ability,
		FSkillHitRuntime& OutRuntime);


};
