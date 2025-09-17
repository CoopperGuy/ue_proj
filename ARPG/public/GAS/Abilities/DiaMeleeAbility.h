#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaMeleeAbility.generated.h"

class UNiagaraSystem;
class USoundBase;

/**
 * 근접 공격 GameplayAbility 클래스
 */
UCLASS()
class ARPG_API UDiaMeleeAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()

public:
	UDiaMeleeAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// 피격 판정 수행
	UFUNCTION(BlueprintCallable, Category = "Melee")
	void PerformHitDetection();

	// 애니메이션 노티파이에서 호출될 함수
	UFUNCTION(BlueprintCallable, Category = "Melee")
	void OnMeleeHitFrame();

	// 데미지 적용
	UFUNCTION(BlueprintCallable, Category = "Melee")
	void ApplyDamageToTarget(AActor* Target);

	// 공격 범위
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Settings")
	float AttackRange = 200.0f;

	// 공격 각도 (도 단위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Settings")
	float AttackAngle = 60.0f;

	// 공격 높이 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Settings")
	FVector AttackOffset = FVector(100.0f, 0.0f, 0.0f);

	// 피격 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Effects")
	UNiagaraSystem* HitEffect;

	// 피격 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Effects")
	USoundBase* HitSound;

	// 피격 판정 디버그 표시 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Debug")
	bool bShowDebugShape = true;

private:
	// 이미 피격된 액터 목록 (ability activation마다 초기화)
	UPROPERTY()
	TArray<AActor*> HitActors;
};