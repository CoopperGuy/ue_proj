// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DiaBaseCharacter.h"
#include "Types/DiaMonsterTable.h"
#include "DiaMonster.generated.h"

class UNiagaraSystem;
class USoundBase;
class UAnimMontage;
class UDiaCombatComponent;
UCLASS()
class ARPG_API ADiaMonster : public ADiaBaseCharacter
{
	GENERATED_BODY()

public:
	ADiaMonster();

    /// <summary>
    /// 사망 관련 
    /// </summary>
    virtual void PlayDieAnimation();

    virtual void Die();

    /// <summary>
	/// UI 관련 함수 
    /// </summary>
    virtual void UpdateHPGauge(float CurHealth, float MaxHelath);

	/////////////////////////////////
    /// 몬스터 생성시 초기화
	/// @param MonsterInfo : 몬스터 정보
	//////////////////////////////////
	void InitializeFromData(const FMonsterInfo& MonsterInfo);

    void ActivateAI();
    void DeactivateAI();
    void ResetMonster();

protected:
	/// <summary>
	/// 엔진 기본 기능 
	/// </summary>
	virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

	/// <summary>
    /// 전투 관련 함수
	/// </summary>
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
public:	

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* hitEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    USoundBase* hitSound;

    // ��Ʈ ���׼� ��Ÿ��
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* hitReactionMontage;
};
