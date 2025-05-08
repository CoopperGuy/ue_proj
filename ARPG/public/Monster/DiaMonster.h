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

/**
 * 몬스터 캐릭터 클래스
 * 몬스터의 상태, AI 활성화/비활성화, 풀링 시스템과의 상호작용 담당
 */
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
    
    /**
     * 몬스터의 중력 활성화/비활성화
     * @param bEnableGravity 중력 활성화 여부
     */
    void SetGravity(bool bEnableGravity);
    
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
    UPROPERTY()
    FName MonsterID;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* hitEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    USoundBase* hitSound;

    // 히트 리액션 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* hitReactionMontage;
};
