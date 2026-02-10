// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "DiaSkillVariantExecutor.generated.h"

class UDiaGameplayAbility;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaSkillVariantExecutor : public UObject
{
	GENERATED_BODY()
public:
	UDiaSkillVariantExecutor();
	virtual void InitializeExecutor();

	void ExecuteEffect(
		const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability);

protected:
	UPROPERTY()
	TMap<FGameplayTag, UDiaSkillVariantEffect*> EffectsByTag;

	// Strategy apply
	virtual void ApplyEffects(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, FSkillSpawnRuntime& Runtime);
	virtual void ApplyEffects(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, FSkillHitRuntime& Runtime);
};
