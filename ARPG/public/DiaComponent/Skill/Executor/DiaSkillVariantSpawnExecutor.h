// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/DiaSkillVariantExecutor.h"
#include "DiaSkillVariantSpawnExecutor.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaSkillVariantSpawnExecutor : public UDiaSkillVariantExecutor
{
	GENERATED_BODY()
	
public:
	virtual void InitializeExecutor() override;

	virtual void ExecuteEffect(
		const TArray<class UDiaSkillVariant*>& Variants,
		FDiaSkillVariantContext& Context,
		class UDiaGameplayAbility* Ability
	);
	
};
