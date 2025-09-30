#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaAttributeSet.h"

#include "AbilitySystemComponent.h"

#include "GameplayEffectTypes.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

#include "GAS/Effects/DiaGE_CoolDown_Generic.h"
#include "GAS/Effects/DiaGE_ManaCost_Generic.h"
#include "GAS/Effects/DiaGameplayEffect_Damage.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "GameplayTagContainer.h"

UDiaGameplayAbility::UDiaGameplayAbility()
{
	// Set default values
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	
	// Set default activation requirements - 신규 API 사용
	{
		//FGameplayTagContainer Tags;
		//Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));
		//SetAssetTags(Tags);
	}
	
	CurrentAbilityMontage = nullptr;
	MontageTask = nullptr;

	ManaCostEffectClass = UDiaGE_ManaCost_Generic::StaticClass();
	CooldownEffectClass = UDiaGE_CoolDown_Generic::StaticClass();
	DamageEffectClass = UDiaGameplayEffect_Damage::StaticClass();
}

void UDiaGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UE_LOG(LogTemp, Warning, TEXT("Activate: Info=%p Owner=%s Avatar=%s"),
		ActorInfo,
		ActorInfo && ActorInfo->OwnerActor.IsValid() ? *ActorInfo->OwnerActor->GetName() : TEXT("null"),
		ActorInfo && ActorInfo->AvatarActor.IsValid() ? *ActorInfo->AvatarActor->GetName() : TEXT("null"));

	// 여기서 null이면 Init 타이밍/ASC 중복/포인터 엇갈림
	check(ActorInfo && ActorInfo->AbilitySystemComponent.IsValid());


    // 안전 가드: ActorInfo가 없으면 종료
    if (!ActorInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("ActivateAbility: Missing ActorInfo"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    auto* ASC = ActorInfo->AbilitySystemComponent.Get();
    auto* Avatar = ActorInfo->AvatarActor.Get();
    auto* Owner = ActorInfo->OwnerActor.Get();

	// 필요한 포인터 null 방어
	if (!ASC || !Avatar || !Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateAbility Failed"));

		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}


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

}

void UDiaGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Stop montage if playing
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDiaGameplayAbility::ApplyDamageToASC(UAbilitySystemComponent* TargetASC, float BaseDamage, float CritMultiplier) const
{
    if (!TargetASC || !DamageEffectClass) return;

    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!SourceASC) return;

    FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
    EffectContext.AddInstigator(CurrentActorInfo ? CurrentActorInfo->OwnerActor.Get() : nullptr,
        CurrentActorInfo ? Cast<APawn>(CurrentActorInfo->AvatarActor.Get()) : nullptr);

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);
    if (!SpecHandle.IsValid()) return;

    SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("GASData.DamageBase")), BaseDamage);
    SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("GASData.CritMultiplier")), CritMultiplier);

    SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
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
	
	if (SkillData.AbilityTags.Num() > 0)
	{
		AbilityTags.AppendTags(SkillData.AbilityTags);
	}

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
	const float manaCost = SkillData.ManaCost; // GASSkillData에서 읽기
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const float currentMana = ASC->GetNumericAttribute(UDiaAttributeSet::GetManaAttribute());
	const bool ok = currentMana >= manaCost;
	if (!ok && OptionalRelevantTags)
	{
		OptionalRelevantTags->AddTag(FGameplayTag::RequestGameplayTag(TEXT("Ability.Cost.Mana.NotEnough")));
	}

	return ok;
}

void UDiaGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!ManaCostEffectClass) return;
	const float manaCost = SkillData.ManaCost; // GASSkillData

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ManaCostEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
	if (auto* Spec = SpecHandle.Data.Get())
	{
		Spec->SetSetByCallerMagnitude(SkillData.ManaCostTags, -manaCost);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}

void UDiaGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownEffectClass) return;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CooldownEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
	if (auto* Spec = SpecHandle.Data.Get())
	{
		// 쿨타임 시간 설정
		Spec->SetSetByCallerMagnitude(SkillData.CoolDownTags, SkillData.CooldownDuration);
		ASC->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}