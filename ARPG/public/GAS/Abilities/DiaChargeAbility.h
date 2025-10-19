// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaChargeAbility.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaChargeAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDiaChargeAbility();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	void UpdateCharge();
	FVector CalcSweepPosition(const FGameplayAbilityActorInfo* ActorInfo);
private:
	FTimerHandle ChargeTimerHandle;

	FVector StartLoc;
	FVector EndLoc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS|Ability", meta = (AllowPrivateAccess = "true"))
	float ChargeDelay = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS|Ability", meta = (AllowPrivateAccess = "true"))
	float ElapsedTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS|Ability", meta = (AllowPrivateAccess = "true"))
	float UpdateTime = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS|Ability", meta = (AllowPrivateAccess = "true"))
	float ChargeDuration = 0.1f; //보통 끝 부분으로 바로 이동.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GAS|Ability", meta = (AllowPrivateAccess = "true"))
	float ChargeDist = 600.f;

};
