// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaGroundAbility.generated.h"

class ADiaSkillActor;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaGroundAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void InitializeWithSkillData(const FGASSkillData& InSkillData) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	UFUNCTION()
	void OnSpawned(AActor* SpawnedSkillGround);
protected:
		// 발사체 생성 함수
	UFUNCTION(BlueprintCallable, Category = "Ground")
	void SpawnSkillGround();

	// 발사체 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<ADiaSkillActor> SkillGroundClass;

	// 소유자 방향으로 발사할지 여부 (false면 마우스 커서 방향)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bUseOwnerRotation = false;	
};
