#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UDiaGameplayAbility::UDiaGameplayAbility()
{
	// Set default values
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// Set default activation requirements
	FGameplayTagContainer Tags;
	Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));
	SetAssetTags(Tags);
	
	CurrentAbilityMontage = nullptr;
	MontageTask = nullptr;

}

void UDiaGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Play animation if available
	if (AbilityMontage)
	{
		PlayAbilityMontage(AbilityMontage);
	}

	// Play visual effects
	if (AbilityEffect && ActorInfo->AvatarActor.IsValid())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			AbilityEffect,
			ActorInfo->AvatarActor->GetActorLocation()
		);
	}

	// Execute ability logic
	OnAbilityExecute();
}

void UDiaGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop montage if playing
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	// Call blueprint event
	OnAbilityEnd();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UDiaGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// Check mana cost
	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		const UDiaAttributeSet* AttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UDiaAttributeSet>();
		if (AttributeSet)
		{
			float CurrentMana = AttributeSet->GetMana();
			if (CurrentMana < SkillData.ManaCost)
			{
				return false;
			}
		}
	}

	return true;
}

void UDiaGameplayAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo != nullptr && ActorInfo->AvatarActor != nullptr)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
	}
}

void UDiaGameplayAbility::InitializeWithSkillData(const FGASSkillData& InSkillData)
{
	SkillData = InSkillData;
	
	// 태그 합치기
	if (SkillData.AbilityTags.Num() > 0)
	{
		FGameplayTagContainer Tags = GetAssetTags();
		Tags.AppendTags(SkillData.AbilityTags);
		SetAssetTags(Tags);
	}

	// 소프트 레퍼런스 동기 로드
	//if (SkillData.CastAnimation.IsValid())
	{
		AbilityMontage = SkillData.CastAnimation.LoadSynchronous();
		//로드 성공 시 로그 출력
#if WITH_EDITOR
		if (AbilityMontage)
		{
			UE_LOG(LogTemp, Log, TEXT("Loaded AbilityMontage: %s"), *AbilityMontage->GetName());
		}
		//else
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillData.CastAnimation is not valid"));
		}

#endif
	}
	//if (SkillData.CastEffect.IsValid())
	{
		AbilityEffect = SkillData.CastEffect.LoadSynchronous();
	}
	{
		LagacyAbilityEffect = SkillData.LegacyCastEffect.LoadSynchronous();
	}
}

float UDiaGameplayAbility::PlayAbilityMontage(UAnimMontage* MontageToPlay, float PlayRate)
{
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!MontageToPlay || !ActorInfo.AvatarActor.IsValid())
	{
		return 0.0f;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get());
	if (!Character)
	{
		return 0.0f;
	}

	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return 0.0f;
	}

	// Stop current montage if playing
	if (MontageTask)
	{
		MontageTask->EndTask();
	}

	// Create and start montage task
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		FName("AbilityMontage"),
		MontageToPlay,
		PlayRate
	);

	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UDiaGameplayAbility::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UDiaGameplayAbility::OnMontageCancelled);
		MontageTask->OnInterrupted.AddDynamic(this, &UDiaGameplayAbility::OnMontageCancelled);
		MontageTask->ReadyForActivation();
	}

	CurrentAbilityMontage = MontageToPlay;
	return MontageToPlay->GetPlayLength() / PlayRate;
}

void UDiaGameplayAbility::StopAbilityMontage(float BlendOutTime)
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
	CurrentAbilityMontage = nullptr;
}

void UDiaGameplayAbility::OnMontageCompleted()
{
	MontageTask = nullptr;
	CurrentAbilityMontage = nullptr;
	
	// End ability when montage completes
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDiaGameplayAbility::OnMontageCancelled()
{
	MontageTask = nullptr;
	CurrentAbilityMontage = nullptr;
	
	// End ability when montage is cancelled
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

bool UDiaGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	const UDiaAttributeSet* AttributeSet = ActorInfo->AbilitySystemComponent->GetSet<UDiaAttributeSet>();
	if (!AttributeSet)
	{
		return false;
	}

	// Check mana cost
	return AttributeSet->GetMana() >= SkillData.ManaCost;
}

void UDiaGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const UDiaAttributeSet* AttributeSet = ASC->GetSet<UDiaAttributeSet>();
	
	if (!AttributeSet)
	{
		return;
	}

	// Apply mana cost directly for now
	// In a more complete implementation, this would use GameplayEffects
	float CurrentMana = AttributeSet->GetMana();
	float NewMana = FMath::Max(0.0f, CurrentMana - SkillData.ManaCost);
	
	// This is a direct attribute modification - normally you'd use GameplayEffects
	const_cast<UDiaAttributeSet*>(AttributeSet)->SetMana(NewMana);
}