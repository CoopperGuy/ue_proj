// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Effects/DiaGameplayEffect.h"
#include "DiaGE_StatusEffect.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ARPG_API UDiaGE_StatusEffect : public UDiaGameplayEffect
{
	GENERATED_BODY()
	
public:
	UDiaGE_StatusEffect();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect Settings")
	float DefaultDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect Settings")
	FGameplayTag StatusTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect Settings")
	FGameplayTagContainer BlockedAbilityTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect Settings")
	bool bAllowStacking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect Settings", meta = (EditCondition = "bAllowStacking"))
	int32 MaxStackCount;
};
