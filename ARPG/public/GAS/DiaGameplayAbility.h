#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"

#include "Types/DiaGASSkillData.h"
#include "GAS/Executor/DiaSkillStepExecutor.h"
#include "DiaGameplayAbility.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class UParticleSystemComponent;
class UNiagaraComponent;
class UAbilitySystemComponent;
class UDiaSkillVariant;
class USkillObject;
class ADiaBaseCharacter;
class ADiaSkillActor;

/**
 * 신규 스킬의 공통 실행 Ability. 단일 동작도 FGASSkillData::Steps에 한 단계로 작성한다.
 * 실행 순서는 RunNextStep과 UDiaSkillStepExecutor가 담당한다.
 * 새 동작은 StepData/Executor를 확장하고, Abilities/의 레거시 전용 Ability를 새로 선택/상속하는 방식은 지양한다.
 */
UCLASS(Blueprintable)
class ARPG_API UDiaGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDiaGameplayAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	//CommitAbility 마나및 코스트 체크 관련 로직이다.
	//그러나 c++로 진행중이기 때문에 activateability에서 이미 commitability를 호출한다.
	//그렇기 때문에 핵심 코어인 apply cost와 checkcost만 오버라이드 한다.
	//protected:에 존재

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Initialize ability with skill data
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void InitializeWithSkillData(const FGASSkillData& InSkillData);

	// Get skill data
	UFUNCTION(BlueprintPure, Category = "Skill")
	const FGASSkillData& GetSkillData() const { return SkillData; }

	void SetSkillObject(const USkillObject* InSkillObject);
	const USkillObject* GetSkillObject() const { return SkillObject; }

	/** Steps에서 첫 유효한 스폰 LifeSpan을 찾고, 없으면 레거시 ExtraData를 확인한다. 양수면 월드 제거 타이머를 적용한다. 스폰 직후 호출. */
	void ApplySkillObjectRemovalTimer(class ADiaSkillActor* SkillActor) const;

	const TSubclassOf<UGameplayEffect> GetDamageEffectClass() const { return DamageEffectClass; }
	virtual TSubclassOf<ADiaSkillActor> GetSkillActorClassForSpawn() const;

	void MakeEffectSpecContextToTarget(TArray<FGameplayEffectSpecHandle>& OutContext) const;
	void MakeEffectSpecContextToSelf(TArray<FGameplayEffectSpecHandle>& OutContext) const;

	void ActiveModifierEffect(ADiaBaseCharacter* Owner);
	ADiaSkillActor* SpawnCosmeticSkillActorAtTransform(TSubclassOf<ADiaSkillActor> SkillActorClass, const FTransform& SpawnTransform, bool bAttachToAvatar = false);
	void ApplySkillDamageToActor(AActor* TargetActor, float CritMultiplier = 1.0f) const;

protected:
	const float CalculateFinalManaCost(const FGameplayAbilityActorInfo* ActorInfo) const;
	const float CalculateFinalCooldown(const FGameplayAbilityActorInfo* ActorInfo, const float BaseCoolDown) const;
protected:
	// Skill data from GAS system
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FGASSkillData SkillData;

	// Animation montage to play
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* AbilityMontage;

	// Visual effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UNiagaraSystem* AbilityEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* LagacyAbilityEffect;

	UPROPERTY(EditDefaultsOnly, Category="GAS|GE")
	TSubclassOf<UGameplayEffect> ManaCostEffectClass; 

	UPROPERTY(EditDefaultsOnly, Category="GAS|GE")
	TSubclassOf<UGameplayEffect> CooldownEffectClass; 

	UPROPERTY(EditDefaultsOnly, Category = "GAS|GE")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 피격 이펙트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Effects")
	UNiagaraSystem* HitEffect;

	// 피격 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Effects")
	USoundBase* HitSound;

	UPROPERTY()
	TArray<TObjectPtr<UDiaSkillStepExecutor>> StepExecutors;

	// Helper functions
	UFUNCTION(BlueprintCallable, Category = "Animation")
	float PlayAbilityMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void StopAbilityMontage(float BlendOutTime = 0.2f);

	UFUNCTION()
	virtual void ProcessSkillDelayEvents();

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	virtual bool ShouldEndAbilityOnMontageCompleted() const;

	/**
	 * 히트 이펙트 스폰 (오버라이드 가능)
	 * @param Location 이펙트 생성 위치
	 * @param HitIndex 현재 히트 인덱스
	 */
	virtual void SpawnHitEffectAtLocation(const FVector& Location);

	/**
	 * 히트 사운드 재생 (오버라이드 가능)
	 * @param Location 사운드 재생 위치
	 * @param HitIndex 현재 히트 인덱스
	 */
	virtual void PlayHitSoundAtLocation(const FVector& Location);


	// Cost and cooldown checks
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual bool CheckCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	// GAS 대미지 적용 헬퍼: TargetASC에 DamageEffectClass를 통해 대미지 적용
	UFUNCTION(BlueprintCallable, Category = "GAS|Damage")
	void ApplyDamageToASC(UAbilitySystemComponent* TargetASC, float BaseDamage, float CritMultiplier = 1.0f) const;

	virtual void ApplyGameplayEffectToTarget(UAbilitySystemComponent* TargetASC) const;
	virtual void ApplyGameplayEffectToSelf() const;

	void ExecuteAbilityLogic(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	UPROPERTY()
	FDiaSkillExecutionContext ExecutionContext;

	int32 CurrentStepIndex = 0;
	void RunNextStep();
	UFUNCTION()
	void ExecuteCurrentStep();
	void HandleCurrentStepFinished();

protected:
	// Current playing montage (renamed to avoid shadowing)
	UPROPERTY()
	UAnimMontage* CurrentAbilityMontage;

	// Montage task handle
	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* MontageTask;

	UPROPERTY()
	class UAbilityTask_SpawnActor* SpawnActorTask;

	// Multi Hit 관련 변수 (근접 공격, 원거리는 Skill자체에 넣어준다.)
	FTimerHandle MultiHitTimerHandle;
	
	int32 CurrentHitCount;

	int32 TotalHitCount;

	float HitInterval;

	bool bSkillExecutionFinished = false;
	bool bOwnsMonsterAttack = false;
	//히트 계산 식
	//-> CurrentHitcount * HitInterval = 총 사용 시간.


	UPROPERTY()
	const USkillObject* SkillObject;

	UPROPERTY()
	FSkillModifierRuntime ModifierRuntime;
};
