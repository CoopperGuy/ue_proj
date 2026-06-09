// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/DiaSkillDelegates.h"
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
	using UDiaSkillVariantExecutor::ExecuteEffect;

	virtual void InitializeExecutor() override;

	virtual void ExecuteEffect(
		const TArray<class UDiaSkillVariant*>& Variants,
		FDiaSkillVariantContext& Context,
		class UDiaGameplayAbility* Ability
	);

	void ExecuteEffect(
		const TArray<class UDiaSkillVariant*>& Variants,
		FDiaSkillVariantContext& Context,
		class UDiaGameplayAbility* Ability,
		FDiaSkillSpawnFinishedDelegate OnFinished
	);
	
	void SpawnCast(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability, FSkillSpawnRuntime& Runtime) const;
	bool AfterSpawn(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability, FSkillSpawnRuntime& Runtime, FDiaSkillSpawnFinishedDelegate OnFinished);

private:
	UPROPERTY()
	FTimerHandle RepeatTimerHandle;
};
