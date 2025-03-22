// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/Combatable.h"
#include "DiaBaseCharacter.h"
#include "DiaMonster.generated.h"

class UNiagaraSystem;
class USoundBase;
class UAnimMontage;
class UDiaCombatComponent;
UCLASS()
class ARPG_API ADiaMonster : public ADiaBaseCharacter, public ICombatable
{
	GENERATED_BODY()

public:
	ADiaMonster();
    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual void PlayDieAnimation();
protected:
	virtual void BeginPlay() override;

    // ���� ���� ���� üũ
    virtual bool CanAttack() const;

    // ���� ���� ���� üũ
    virtual bool IsInCombat() const;
private:
    // ��� ó��
    virtual void Die();
public:	

	// ������ ó�� �Լ�
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    virtual void UpdateHPGauge(float CurHealth, float MaxHelath);
protected:
    // �ǰ� ����Ʈ
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    UNiagaraSystem* hitEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    USoundBase* hitSound;

    // ��Ʈ ���׼� ��Ÿ��
    UPROPERTY(EditDefaultsOnly, Category = "Animation")
    UAnimMontage* hitReactionMontage;

    // 몬스터 스킬 ID 목록은 유지
    UPROPERTY(EditDefaultsOnly, Category = "Skills")
    TArray<int32> MonsterSkills;
};
