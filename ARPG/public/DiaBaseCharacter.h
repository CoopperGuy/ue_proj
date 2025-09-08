// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "DiaComponent/DiaStatusEffectComponent.h"
#include "DiaBaseCharacter.generated.h"

class UDiaCombatComponent;
class UAnimMontage;
class UDiaStatComponent;
class UDiaStatusEffectComponent;
class UAbilitySystemComponent;
class UDiaAttributeSet;

UCLASS()
class ARPG_API ADiaBaseCharacter : public ACharacter, public IAbilitySystemInterface
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
	virtual void Die();

	void AddExp(float ExpAmount);

	void SetGravity(bool bEnableGravityAndCollision);
protected:
	// 기본적인 함수
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/// <summary>
	/// 스킬 관련 함수 및 초기화
	/// </summary>
	virtual void SetupInitialSkills();

	virtual void GrantInitialGASAbilities();
protected:
	//전투 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stat)
    UDiaCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stat)
    UDiaStatComponent* StatsComponent;	

	// 상태 이상 효과 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffects")
	UDiaStatusEffectComponent* StatusEffectComponent;

	// GAS Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UDiaAttributeSet* AttributeSet;

	//초기 보유 스킬
	UPROPERTY(EditAnywhere, Category = "Skills")
    TArray<int32> InitialSkills;

	// 현재 재생중인 몽타주
	UPROPERTY()
	UAnimMontage* CurrentMontage;

public:
	UDiaStatComponent* GetStatComponent() const { return StatsComponent; }
	UDiaCombatComponent* GetCombatComponent() const { return CombatComponent; }
	UDiaAttributeSet* GetAttributeSet() const { return AttributeSet; }
};
