// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaDodgeAbility.generated.h"

/**
 * 레거시 회피 Ability. 기존 데이터/BP 호환을 위해 유지하며 신규 사용은 지양한다.
 * 신규 스킬은 UDiaGameplayAbility + FGASSkillData::Steps로 작성하고, 필요한 회피 동작은 StepData/Executor를 확장한다.
 * 이 클래스는 자체 ActivateAbility 경로를 사용하므로 Steps만 채워도 Step 실행으로 전환되지 않는다.
 */
UCLASS()
class ARPG_API UDiaDodgeAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()
public:
	UDiaDodgeAbility();
	virtual void InitializeWithSkillData(const FGASSkillData& InSkillData) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
private:
	UFUNCTION()
	void OnRollFinish();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dodge|Settings", meta = (AllowPrivateAccess = "true"))
	float DodgeStrength = 600.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Dodge|Settings", meta = (AllowPrivateAccess = "true"))
	float DodgeDuration = 0.5f;
};
