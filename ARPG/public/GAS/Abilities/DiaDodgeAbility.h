// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaDodgeAbility.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaDodgeAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()
public:
	UDiaDodgeAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UFUNCTION()
	void OnRollFinish();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dodge|Settings", meta = (AllowPrivateAccess = "true"))
	float DodgeStrength = 600.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dodge|Settings", meta = (AllowPrivateAccess = "true"))
	float DodgeDuration = 0.5f;
};
