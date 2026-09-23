#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaMeleeAbility.generated.h"

class UNiagaraSystem;
class USoundBase;

/**
 * 레거시 근접 공격 Ability. 기존 데이터/BP 호환을 위해 유지하며 신규 사용은 지양한다.
 * 신규 스킬은 UDiaGameplayAbility + FGASSkillData::Steps의 FGASMeleeSpawnStepData를 우선 사용한다.
 * Steps가 있는 기존 데이터는 공통 Step 실행기로 위임하고, Steps가 없을 때만 이전 타격 경로를 사용한다.
 */
UCLASS()
class ARPG_API UDiaMeleeAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()

public:
	UDiaMeleeAbility();

	virtual void InitializeWithSkillData(const FGASSkillData& InSkillData) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 피격 판정 수행 (단일 히트용)
	UFUNCTION(BlueprintCallable, Category = "Melee")
	void PerformHitDetection();

	// 애니메이션 노티파이에서 호출될 함수
	UFUNCTION(BlueprintCallable, Category = "Melee")
	void OnMeleeHitFrame();

	// 데미지 적용
	UFUNCTION(BlueprintCallable, Category = "Melee")
	void ApplyDamageToTarget(AActor* Target);

	// Multi Hit 시스템 시작 (HitCount > 1일 때)
	UFUNCTION(BlueprintCallable, Category = "Melee")
	void StartMultiHit();

	virtual void ProcessSkillDelayEvents() override;
protected:
	// 공격 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Settings")
	float AttackRange = 200.0f;

	// 공격 각도 (도 단위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Settings")
	float AttackAngle = 60.0f;

	// 공격 높이 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Settings")
	FVector AttackOffset = FVector(100.0f, 0.0f, 0.0f);

	// 피격 판정 디버그 표시 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Debug")
	bool bShowDebugShape = true;

private:
	// 다음 히트 진행
	void ProcessNextHit();

	// Multi Hit 타이머 정리
	void ClearMultiHitTimer();

	// 이미 피격된 액터 목록 (각 히트마다 초기화되지 않음 - 한 어빌리티 동안 한 적은 한 번만 맞음)
	UPROPERTY()
	TArray<AActor*> HitActors;

};
