#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

#include "GameplayEffectTypes.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

#include "GAS/Effects/DiaGE_CoolDown_Generic.h"
#include "GAS/Effects/DiaGE_ManaCost_Generic.h"
#include "GAS/Effects/DiaGameplayEffect_Damage.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
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
		UE_LOG(LogTemp, Warning, TEXT("ActivateAbility Failed: Invalid ASC/Avatar/Owner"));
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

void UDiaGameplayAbility::SpawnHitEffectAtLocation(const FVector& Location)
{
	// 히트 이펙트가 설정되어 있으면 재생
	if (HitEffect)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				HitEffect,
				Location
			);
		}
	}
}

void UDiaGameplayAbility::PlayHitSoundAtLocation(const FVector& Location)
{
	// 히트 사운드가 설정되어 있으면 재생
	if (HitSound)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UGameplayStatics::PlaySoundAtLocation(
				World,
				HitSound,
				Location
			);
		}
	}
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
		//FGameplayTagContainer NewTags = GetAssetTags();
		//NewTags.AppendTags(SkillData.AbilityTags);
		//SetAssetTags(NewTags);

	}

	//if (SkillData.CastAnimation.IsValid())
	{
		AbilityMontage = SkillData.CastAnimation.LoadSynchronous();
		//로드 성공 시 로그 출력
#if WITH_EDITOR
		if (AbilityMontage)
		{
			//UE_LOG(LogTemp, Log, TEXT("Loaded AbilityMontage: %s"), *AbilityMontage->GetName());
		}
		//else
		{
			//UE_LOG(LogTemp, Warning, TEXT("SkillData.CastAnimation is not valid"));
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
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid()) return;

	const float manaCost = SkillData.ManaCost;
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ManaCostEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
	if (auto* Spec = SpecHandle.Data.Get())
	{
		// 음수로 적용하여 마나 감소 (Additive이므로 -값을 더하면 감소)
		Spec->SetSetByCallerMagnitude(SkillData.ManaCostTags, -manaCost);

		Spec->DynamicGrantedTags.AddTag(SkillData.ManaCostTags);

		ASC->ApplyGameplayEffectSpecToSelf(*Spec);

		//UE_LOG(LogTemp, Log, TEXT("Applied Mana Cost: -%f (Current Mana: %f)"), manaCost, 
		//	ASC->GetNumericAttribute(UDiaAttributeSet::GetManaAttribute()));
	}
}

bool UDiaGameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ensure(ActorInfo))
	{
		return true;
	}

	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (!AbilitySystemComponent)
	{
		return true;
	}

	// Handle로 Spec를 찾아서 SkillID(InputID) 가져오기
	FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!Spec)
	{
		return true;
	}

	// Spec에서 쿨다운 태그 가져오기 (GrantAbilityFromSkillData에서 저장됨)
	FGameplayTag CooldownTag;
	if (Spec->SetByCallerTagMagnitudes.Num() > 0)
	{
		// SetByCallerTagMagnitudes의 첫 번째 태그가 쿨다운 태그
		for (const auto& Pair : Spec->SetByCallerTagMagnitudes)
		{
			if (Pair.Key.MatchesTag(FGameplayTag::RequestGameplayTag(FName("GASData.CoolDown"), false)))
			{
				CooldownTag = Pair.Key;
				break;
			}
		}
	}
	
	if (CooldownTag.IsValid() && AbilitySystemComponent->HasMatchingGameplayTag(CooldownTag))
	{
		if (OptionalRelevantTags)
		{
			const FGameplayTag& FailCooldownTag = UAbilitySystemGlobals::Get().ActivateFailCooldownTag;
			if (FailCooldownTag.IsValid())
			{
				OptionalRelevantTags->AddTag(FailCooldownTag);
			}

			// 쿨다운 태그 추가
			OptionalRelevantTags->AddTag(CooldownTag);
		}

		return false;
	}
	
	return true;
}


void UDiaGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownEffectClass) return;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return;

	// Handle로 Spec를 찾아서 SkillID(InputID) 가져오기
	FGameplayAbilitySpec* AbilitySpec = ASC->FindAbilitySpecFromHandle(Handle);
	if (!AbilitySpec)
	{
		return;
	}

	// Spec에서 쿨다운 태그 가져오기 (GrantAbilityFromSkillData에서 저장됨)
	FGameplayTag CooldownTag;
	float cooldownDuration = SkillData.CooldownDuration;
	
	if (AbilitySpec->SetByCallerTagMagnitudes.Num() > 0)
	{
		// SetByCallerTagMagnitudes에서 쿨다운 태그와 Duration 가져오기
		for (const auto& Pair : AbilitySpec->SetByCallerTagMagnitudes)
		{
			if (Pair.Key.MatchesTag(FGameplayTag::RequestGameplayTag(FName("GASData.CoolDown"), false)))
			{
				CooldownTag = Pair.Key;
				cooldownDuration = Pair.Value; // Spec에 저장된 Duration 사용
				break;
			}
		}
	}
	
	if (!CooldownTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[ApplyCooldown] No cooldown tag found in Spec for SkillID: %d"), AbilitySpec->InputID);
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CooldownEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
	if (auto* Spec = SpecHandle.Data.Get())
	{
		// ★ Duration은 GASData.CoolDown 태그로 설정 (DiaGE_CoolDown_Generic에서 하드코딩됨)
		FGameplayTag DurationTag = FGameplayTag::RequestGameplayTag(FName("GASData.CoolDown"), false);
		Spec->SetSetByCallerMagnitude(DurationTag, cooldownDuration);
		
		// ★ 쿨다운 태그는 스킬별 태그로 부여 (GASData.CoolDown.2001 등)
		Spec->DynamicGrantedTags.AddTag(CooldownTag);
		
		// 적용
		ASC->ApplyGameplayEffectSpecToSelf(*Spec);
		
		//UE_LOG(LogTemp, Log, TEXT("[ApplyCooldown] Applied cooldown for SkillID: %d, Duration: %.2f, Tag: %s"), 
		//	AbilitySpec->InputID, cooldownDuration, *CooldownTag.ToString());
	}
}