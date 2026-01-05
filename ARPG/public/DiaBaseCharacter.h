// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "AbilitySystemInterface.h"
#include "Interface/Damageable.h"

#include "DiaComponent/DiaStatusEffectComponent.h"

#include "GameplayTagContainer.h"

#include "DiaBaseCharacter.generated.h"

class UDiaCombatComponent;
class UAnimMontage;
class UDiaStatusEffectComponent;
class UAbilitySystemComponent;
class UDiaAttributeSet;
class UDiaLevelComponent;
class UDiaSkillManagerComponent;
UCLASS()
class ARPG_API ADiaBaseCharacter : public ACharacter, public IAbilitySystemInterface, public IDamageable
{
	GENERATED_BODY()

public:
	ADiaBaseCharacter();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	//////////////////////////////////////////////////////////////////////////
	// Animation Montage
	// Montage 재생, 정지, 체크
	// Montage 종료 시 delegate 처리
	//////////////////////////////////////////////////////////////////////////
	UFUNCTION(Category = "Animation")
	float PlayCharacterMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.0f);

	// 현재 재생중인 몽타주 중단
	UFUNCTION(Category = "Animation")
	void StopCharacterMontage(float BlendOutTime = 0.2f);

	UFUNCTION(Category = "Animation")
	bool IsPlayingMontage(UAnimMontage* Montage) const;
	
	virtual void PlayDieAnimation();

	virtual void StopAnimMontage(UAnimMontage* Montage) override;

	UFUNCTION()
	virtual void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	virtual void SetTargetActor(ADiaBaseCharacter* NewTarget);

	/// <summary>
	/// UI 관련 함수
	/// UI Update용 함수 
	/// 차후 SPGuage 추가 가능함
	/// </summary>
	/// <param name="CurHealth"></param>
	/// <param name="MaxHelath"></param>
	virtual void UpdateHPGauge(float CurHealth, float MaxHelath);

	/// <summary>
	/// 전투 관련 함수 
	/// </summary>
	/// CombatStatCompent를 통해 데미지를 받음
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	//사망 처리
	virtual void Die(ADiaBaseCharacter* Causer);

	void SetGravity(bool bEnableGravityAndCollision);

	virtual void OnLevelUp();

	void PauseCurrentMontage();
	void PauseDeathMontage();
protected:
	// 기본적인 함수
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/// <summary>
	/// 스킬 관련 함수 및 초기화
	/// </summary>
	virtual void SetupInitialSkills();

	virtual void GrantInitialGASAbilities();

	virtual bool SetUpSkillID(int32 SkillID);

/// <summary>
/// 상태 이상
/// </summary>
	UFUNCTION()
	virtual void OnStunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION()
	virtual void OnSlowTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
protected:
	// 상태 이상 효과 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffects")
	UDiaLevelComponent* LevelComponent;

	// GAS Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UDiaAttributeSet* AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UDiaSkillManagerComponent* SkillManagerComponent;

	// 현재 재생중인 몽타주
	UPROPERTY()
	UAnimMontage* CurrentMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Ability")
    FGameplayTagContainer AbilityTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Ability")
	int32 DodgeSkillID = 1011;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	UAnimMontage* StunMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ability")
	UAnimMontage* DieMontage;

	TWeakObjectPtr<ADiaBaseCharacter> KillerCharacterWeakPtr;

	bool bIsDead{ false };

	float DefaultMovementSpeed{ 600.f };
public:
	UDiaAttributeSet* GetAttributeSet() const { return AttributeSet; }
	bool GetIsDead() const { return bIsDead; }
	UDiaSkillManagerComponent* GetSkillManagerComponent() const { return SkillManagerComponent; }
};
