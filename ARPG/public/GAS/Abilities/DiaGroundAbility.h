// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaGroundAbility.generated.h"

class ADiaSkillActor;
/**
 * 레거시 장판 Ability. 기존 데이터/BP 호환을 위해 유지하며 신규 사용은 지양한다.
 * 신규 스킬은 UDiaGameplayAbility + FGASSkillData::Steps의 FGASGroundSpawnStepData를 우선 사용한다.
 * Steps가 있는 기존 데이터는 공통 Step 실행기로 위임한다. ADiaGroundObj Actor는 Step에서도 계속 사용한다.
 */
UCLASS()
class ARPG_API UDiaGroundAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void InitializeWithSkillData(const FGASSkillData& InSkillData) override;
	virtual TSubclassOf<ADiaSkillActor> GetSkillActorClassForSpawn() const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UFUNCTION()
	void OnSpawned(AActor* SpawnedSkillGround);
protected:
		// 발사체 생성 함수
	UFUNCTION(BlueprintCallable, Category = "Ground")
	void SpawnSkillGround();

	virtual void ProcessSkillDelayEvents() override;
	virtual bool ShouldEndAbilityOnMontageCompleted() const override;
protected:
	// 발사체 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<ADiaSkillActor> SkillGroundClass;

	// 소유자 방향으로 발사할지 여부 (false면 마우스 커서 방향)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bUseOwnerRotation = false;	
};
