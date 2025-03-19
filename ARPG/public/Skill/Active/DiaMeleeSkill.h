// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/DiaSkillBase.h"
#include "DiaMeleeSkill.generated.h"

/**
 * 근접 공격 스킬 클래스
 */
UCLASS()
class ARPG_API ADiaMeleeSkill : public ADiaSkillBase
{
	GENERATED_BODY()
	
public:
	ADiaMeleeSkill();

	virtual void ExecuteSkill() override;
	virtual void EndSkill() override;

protected:
	virtual void OnSkillStart() override;
	virtual void OnSkillEnd() override;

	// 피격 판정 수행
	UFUNCTION()
	void PerformHitDetection();

	// 애니메이션 노티파이 이벤트 처리
	UFUNCTION(BlueprintCallable, Category = "Skill|Animation")
	void OnMeleeHitFrame();

	// 데미지 적용
	void ApplyDamage(AActor* Target);

protected:
	// 공격 범위
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Settings")
	float AttackRange = 200.0f;

	// 공격 각도 (도 단위)
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Settings")
	float AttackAngle = 60.0f;

	// 공격 높이 오프셋
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Settings")
	FVector AttackOffset = FVector(100.0f, 0.0f, 0.0f);

	// 이미 피격된 액터 목록
	UPROPERTY()
	TArray<AActor*> HitActors;

	// 피격 판정 디버그 표시 여부
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Debug")
	bool bShowDebugShape = false;
	
	// 피격 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Effects")
	UNiagaraSystem* HitEffect;
	
	// 피격 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Effects")
	USoundBase* HitSound;
};
