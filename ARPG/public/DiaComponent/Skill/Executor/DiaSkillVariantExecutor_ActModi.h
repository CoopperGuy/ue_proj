// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/DiaSkillVariantExecutor.h"
#include "DiaSkillVariantExecutor_ActModi.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaSkillVariantExecutor_ActModi : public UDiaSkillVariantExecutor
{
	GENERATED_BODY()
	
public:
	virtual void InitializeExecutor();

	void ExecuteEffect(
		const TArray<class UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, const UDiaGameplayAbility* Ability,
		FSkillVariantRuntime& OutRuntime);
};
