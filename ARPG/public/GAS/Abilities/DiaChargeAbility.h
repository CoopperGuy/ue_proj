// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaChargeAbility.generated.h"

class UCurveVector;
/**
 * 레거시 돌진 Ability. 기존 데이터/BP 호환을 위해 유지하며 신규 사용은 지양한다.
 * 신규 스킬은 UDiaGameplayAbility + FGASSkillData::Steps의 FGASChargeStepData를 우선 사용한다.
 * 이 클래스는 자체 ActivateAbility 경로를 사용하므로 Steps만 채워도 Step 실행으로 전환되지 않는다.
 */
UCLASS()
class ARPG_API UDiaChargeAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()
	
public:
	UDiaChargeAbility();
	virtual void InitializeWithSkillData(const FGASSkillData& InSkillData) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	FVector CalcSweepPosition(const FGameplayAbilityActorInfo* ActorInfo);
	void ApplyHitToActorsInPath(AActor* TargetActor);

	UFUNCTION()
	void OnDashFinished();
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

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveVector> PathOffsetCurve;
};
