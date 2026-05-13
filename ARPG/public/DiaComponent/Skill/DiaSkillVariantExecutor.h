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

	virtual void ExecuteEffect(
		const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability);

	virtual void ExecuteEffect(
		const TArray<class UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, const UDiaGameplayAbility* Ability,
		FSkillVariantRuntime& OutRuntime);

protected:
	UPROPERTY()
	TArray<TObjectPtr<UDiaSkillVariantEffect>> RegisteredEffects;

	// Strategy apply
	virtual void ApplyEffects(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime);

};
