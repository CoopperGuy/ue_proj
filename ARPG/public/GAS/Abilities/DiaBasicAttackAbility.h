#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaBasicAttackAbility.generated.h"

/**
 * 레거시 기본 공격 테스트 구현. 기존 참조 호환을 위해 유지한다.
 * 신규 스킬은 이 클래스를 선택/상속하기보다 UDiaGameplayAbility + FGASSkillData::Steps로 작성한다.
 * 이 클래스는 자체 ActivateAbility 경로를 사용하므로 Steps만 채워도 Step 실행으로 전환되지 않는다.
 */
UCLASS()
class ARPG_API UDiaBasicAttackAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()

public:
	UDiaBasicAttackAbility();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// 공격 실행
	UFUNCTION()
	void PerformAttack();

	// 공격 범위 및 데미지 설정
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackAngle = 90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float BaseDamage = 50.0f;

	// 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AttackMontage;

	// 비용/쿨다운 파라미터
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Cost")
	float ManaCost = 10.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Ability|Cooldown")
	float CooldownDuration = 2.0f;

	// 공격 타이머 핸들
	FTimerHandle AttackTimer;
};
