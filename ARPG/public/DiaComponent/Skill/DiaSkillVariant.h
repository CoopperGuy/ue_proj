// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "DiaSkillVariant.generated.h"


/**
 * 
 */
UCLASS()
class ARPG_API UDiaSkillVariant : public UObject
{
	GENERATED_BODY()
	
public:
	UDiaSkillVariant();

	void InitializeVariant(int32 _VariantID);
	void SkillVariantFunction(); 

	void SkillSpawnFunction();
	void SkillHitFunction();
	void SkillEndFunction();

protected:
	FGameplayTag SkillTag;
	float ModifierValue;

public:
	FORCEINLINE const FGameplayTag& GetSkillTags() const { return SkillTag; }
	FORCEINLINE float GetModifierValue() const { return ModifierValue; }
};
