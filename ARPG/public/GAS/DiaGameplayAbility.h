#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Types/DiaGASSkillData.h"
#include "DiaGameplayAbility.generated.h"

class UAnimMontage;
class UNiagaraSystem;
class UParticleSystemComponent;
class UNiagaraComponent;
UCLASS(Abstract, Blueprintable)
class ARPG_API UDiaGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UDiaGameplayAbility();

	// Override GameplayAbility functions
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	// Initialize ability with skill data
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual void InitializeWithSkillData(const FGASSkillData& InSkillData);

	// Get skill data
	UFUNCTION(BlueprintPure, Category = "Skill")
	const FGASSkillData& GetSkillData() const { return SkillData; }

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

	// Ability execution logic - override in child classes
	UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
	void OnAbilityExecute();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ability")
	void OnAbilityEnd();

	// Helper functions
	UFUNCTION(BlueprintCallable, Category = "Animation")
	float PlayAbilityMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void StopAbilityMontage(float BlendOutTime = 0.2f);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	// Cost and cooldown checks
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

private:
	// Current playing montage (renamed to avoid shadowing)
	UPROPERTY()
	UAnimMontage* CurrentAbilityMontage;

	// Montage task handle
	UPROPERTY()
	class UAbilityTask_PlayMontageAndWait* MontageTask;
};