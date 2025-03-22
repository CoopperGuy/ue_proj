// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiaComponent/DiaStatusEffectComponent.h"
#include "DiaBaseCharacter.generated.h"

class UDiaCombatComponent;
class UAnimMontage;

UCLASS()
class ARPG_API ADiaBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADiaBaseCharacter();

	// 애님 몽타주 재생 함수
	UFUNCTION(Category = "Animation")
	float PlayCharacterMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.0f);

	// 현재 재생중인 몽타주 중단
	UFUNCTION(Category = "Animation")
	void StopCharacterMontage(float BlendOutTime = 0.2f);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool IsPlayingMontage(UAnimMontage* Montage) const;
	
	virtual void StopAnimMontage(UAnimMontage* Montage) override;

	virtual void UpdateHPGauge(float CurHealth, float MaxHelath);

	virtual void PlayDieAnimation();
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	virtual void SetupInitialSkills();

	// ������ ó�� �Լ�
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void Die();
private:
	// 현재 재생중인 몽타주
	UPROPERTY()
	UAnimMontage* CurrentMontage;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Stat)
    UDiaCombatComponent* CombatStatsComponent;	

	UPROPERTY(EditAnywhere, Category = "Skills")
    TArray<int32> InitialSkills;

	UFUNCTION()
	virtual void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 상태 이상 효과 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffects")
	UDiaStatusEffectComponent* StatusEffectComponent;

};
