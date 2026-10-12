#include "GAS/DiaGameplayAbility.h"
#include "GAS/Executor/DiaStepVariantSupport.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGASHelper.h"
#include "GAS/DiaGameplayTags.h"

#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaComponent/Skill/SkillObject.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

#include "GameplayEffectTypes.h"

#include "DiaBaseCharacter.h"
#include "Monster/DiaMonster.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"

#include "GAS/Effects/DiaGE_CoolDown_Generic.h"
#include "GAS/Effects/DiaGE_ManaCost_Generic.h"
#include "GAS/Effects/DiaGameplayEffect_Damage.h"

#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/DiaSkillActorSpawnHelper.h"

#include "Engine/World.h"
#include "Skill/DiaSkillActor.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "GameplayTagContainer.h"
#include "Logging/ARPGLogChannels.h"

namespace
{
float GetSkillActorLifeSpan(const FGASSkillData& SkillData)
{
	for (const FInstancedStruct& Step : SkillData.Steps)
	{
		if (const FGASProjectileSpawnStepData* ProjectileStep = Step.GetPtr<FGASProjectileSpawnStepData>())
		{
			if (ProjectileStep->ProjectileData.LifeSpan > 0.f)
			{
				return ProjectileStep->ProjectileData.LifeSpan;
			}
		}

		if (const FGASGroundSpawnStepData* GroundStep = Step.GetPtr<FGASGroundSpawnStepData>())
		{
			if (GroundStep->GroundData.LifeSpan > 0.f)
			{
				return GroundStep->GroundData.LifeSpan;
			}
		}

		if (const FGASMeleeSpawnStepData* MeleeStep = Step.GetPtr<FGASMeleeSpawnStepData>())
		{
			if (MeleeStep->MeleeData.LifeSpan > 0.f)
			{
				return MeleeStep->MeleeData.LifeSpan;
			}
		}

		if (const FGASUtilityStepData* UtilityStep = Step.GetPtr<FGASUtilityStepData>())
		{
			if (UtilityStep->UtilityData.LifeSpan > 0.f)
			{
				return UtilityStep->UtilityData.LifeSpan;
			}
		}
	}

	if (const FGASUtilitySpawnData* UtilityData = SkillData.GetExtraPtr<FGASUtilitySpawnData>())
	{
		return UtilityData->LifeSpan;
	}
	if (const FGASMeleeData* MeleeData = SkillData.GetExtraPtr<FGASMeleeData>())
	{
		return MeleeData->LifeSpan;
	}
	if (const FGASProjectileData* ProjectileData = SkillData.GetExtraPtr<FGASProjectileData>())
	{
		return ProjectileData->LifeSpan;
	}
	if (const FGASGroundData* GroundData = SkillData.GetExtraPtr<FGASGroundData>())
	{
		return GroundData->LifeSpan;
	}
	if (const FGASSkillActorSpawnData* SpawnData = SkillData.GetExtraPtr<FGASSkillActorSpawnData>())
	{
		return SpawnData->LifeSpan;
	}

	return 0.f;
}

}

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

	ModifierRuntime.CDRP = 1.f;
	ModifierRuntime.MCRP = 1.f;
}

bool UDiaGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}
	if (const ADiaMonster* Monster = Cast<ADiaMonster>(ActorInfo->AvatarActor.Get()); Monster && Monster->IsAttackInProgress())
	{
		return false;
	}

	if (ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
	{
		if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Stunned))
		{
			if (OptionalRelevantTags)
			{
				OptionalRelevantTags->AddTag(FDiaGameplayTags::Get().State_Stunned);
			}
			return false;
		}
	}
	return true;
}

void UDiaGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 안전 가드: ActorInfo가 없으면 종료
	if (!ActorInfo)
	{
		UE_LOG(LogARPG, Warning, TEXT("ActivateAbility: Missing ActorInfo"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	auto* ASC = ActorInfo->AbilitySystemComponent.Get();
	auto* Avatar = ActorInfo->AvatarActor.Get();
	auto* Owner = ActorInfo->OwnerActor.Get();

	// 필요한 포인터 null 방어
	if (!ASC || !Avatar || !Owner)
	{
		UE_LOG(LogARPG, Warning, TEXT("ActivateAbility Failed: Invalid ASC/Avatar/Owner"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//스턴 상태면 패스
	bool bIsStunned = ASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Stunned);
	if (bIsStunned)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Play animation if available
	bSkillExecutionFinished = false;
	CurrentAbilityMontage = nullptr;
	if (ADiaMonster* Monster = Cast<ADiaMonster>(Avatar))
	{
		Monster->BeginAttack();
		bOwnsMonsterAttack = true;
		if (UAnimMontage* MonsterMontage = Monster->GetSkillMontageOverride(SkillData.SkillID))
		{
			AbilityMontage = MonsterMontage;
		}
	}
	if (AbilityMontage)
	{
		float rate = PlayAbilityMontage(AbilityMontage);
		// A failed montage can cancel this ability synchronously.
		if (!IsActive()) return;
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


	if (SkillData.CastTime > 0.f)
	{
		UAbilityTask_WaitDelay* Task = UAbilityTask_WaitDelay::WaitDelay(this, SkillData.CastTime);
		Task->OnFinish.AddDynamic(this, &UDiaGameplayAbility::ProcessSkillDelayEvents);
		Task->ReadyForActivation();
	}
	else
	{
		ProcessSkillDelayEvents();
	}

	ApplyGameplayEffectToSelf();
}

void UDiaGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// A successful hit is not the end of the monster's attack animation. Keep the
	// ability (and BT task) active through recovery. Cancellation never waits.
	if (!bWasCancelled && bOwnsMonsterAttack && CurrentAbilityMontage)
	{
		bSkillExecutionFinished = true;
		return;
	}

	if (bWasCancelled) StopAbilityMontage(0.1f);
	if (!bOwnsMonsterAttack && MontageTask)
	{
		// Preserve player skill montage lifetime; this recovery change is for AI attacks.
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
	// Let Super notify the montage task that its owning ability ended. Calling
	// EndTask here used to detach it first, leaving an orphaned animation playing.
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	MontageTask = nullptr;
	CurrentAbilityMontage = nullptr;
	if (bOwnsMonsterAttack)
	{
		bOwnsMonsterAttack = false;
		if (ADiaMonster* Monster = Cast<ADiaMonster>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr))
			Monster->FinishAttack();
	}
}

void UDiaGameplayAbility::ApplyDamageToASC(UAbilitySystemComponent* TargetASC, float BaseDamage, float CritMultiplier) const
{
	if (!TargetASC || !DamageEffectClass) return;

	// 무적 상태 체크
	if (TargetASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Invincible)) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	
	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);
	if (!SpecHandle.IsValid()) return;

	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("GASData.DamageBase")), BaseDamage);
	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("GASData.CritMultiplier")), CritMultiplier);

	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	ApplyGameplayEffectToTarget(TargetASC);
}

void UDiaGameplayAbility::ApplySkillDamageToActor(AActor* TargetActor, float CritMultiplier) const
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!IsValid(TargetASC))
	{
		return;
	}

	const float PowerScale = DiaStepVariants::Product(DiaStepVariants::Collect(*this), FDiaGameplayTags::Get().GASData_Variant_DamageMultiplier);
	ApplyDamageToASC(TargetASC, SkillData.BaseDamage * PowerScale, CritMultiplier);
	DiaStepVariants::ApplyTargetEffects(*this, TargetASC);
}

const float UDiaGameplayAbility::CalculateFinalManaCost(const FGameplayAbilityActorInfo* ActorInfo) const
{
	const float manaCost = FMath::Max(SkillData.ManaCost * ModifierRuntime.MCRP, 0.f);
	return manaCost;
}

const float UDiaGameplayAbility::CalculateFinalCooldown(const FGameplayAbilityActorInfo* ActorInfo, const float BaseCoolDown) const
{
	const UAbilitySystemComponent* ASC =  ActorInfo->AbilitySystemComponent.Get();

	if (IsValid(ASC))
	{
		const float CooldownReduction = ASC->GetNumericAttribute(UDiaAttributeSet::GetCooldownReductionAttribute());
		const float CooldownReductionMultiplier = FMath::Clamp(1.0f - CooldownReduction * 0.01f, 0.0f, 1.0f);
		const float cooldownDuration = FMath::Max(BaseCoolDown * ModifierRuntime.CDRP * CooldownReductionMultiplier, 0.f); // Duration이 음수인 경우 0으로 보정

		return cooldownDuration;
	}

	return 0.0f;
}

void UDiaGameplayAbility::ApplyGameplayEffectToTarget(UAbilitySystemComponent* TargetASC) const
{
	if (!IsValid(TargetASC))
		return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(SourceASC))
		return;

	for (const auto EffectClass : SkillData.EffectsToApplyOnHit)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();

		UE_LOG(LogARPG, Log, TEXT("Applying Effect: %s"), *EffectClass->GetName());
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), EffectContext);
		if (!SpecHandle.IsValid())
			continue;

		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
		if (Spec)
		{
			// 적용 전 상태
			
			FActiveGameplayEffectHandle ActiveHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*Spec, TargetASC);
			
			// 적용 후 상태
			if (ActiveHandle.IsValid())
			{
				bool bHasStunAfter = TargetASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Stunned);
			}
		}
	}
}

void UDiaGameplayAbility::ApplyGameplayEffectToSelf() const
{
	DiaStepVariants::ApplyCastSelfEffects(*this);
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(SourceASC))
		return;
	for (const auto EffectClass : SkillData.EffectsToApplyOnSelf)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), EffectContext);

		if (!SpecHandle.IsValid())
			return;

		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
		if (Spec)
		{
			FActiveGameplayEffectHandle ActiveHandle = SourceASC->ApplyGameplayEffectSpecToSelf(*Spec);

			if(ActiveHandle.IsValid())
			{
				UE_LOG(LogARPG, Log, TEXT("Applied effect to self: %s"), *EffectClass->GetName());
			}
		}
	}
}

void UDiaGameplayAbility::ExecuteAbilityLogic(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	CurrentStepIndex = 0;
	ExecutionContext = FDiaSkillExecutionContext();
	RunNextStep();
}

void UDiaGameplayAbility::RunNextStep()
{
	if (!IsActive())
	{
		return;
	}
	UE_LOG(LogARPG, Log, TEXT("Running step %d"), CurrentStepIndex);
	if (CurrentStepIndex >= SkillData.Steps.Num())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	const FGASSkillStepDataBase* Step = SkillData.Steps[CurrentStepIndex].GetPtr<FGASSkillStepDataBase>();
	if (Step && Step->StartDelay > 0.f)
	{
		// Ability tasks are cancelled with the ability, so delayed hits cannot survive interruption.
		UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, Step->StartDelay);
		DelayTask->OnFinish.AddDynamic(this, &ThisClass::ExecuteCurrentStep);
		DelayTask->ReadyForActivation();
		return;
	}
	ExecuteCurrentStep();
}

void UDiaGameplayAbility::ExecuteCurrentStep()
{
	if (!IsActive() || !SkillData.Steps.IsValidIndex(CurrentStepIndex))
	{
		return;
	}
	UE_LOG(LogARPG, Log, TEXT("Current step type: %s"), *SkillData.Steps[CurrentStepIndex].GetScriptStruct()->GetName());
	FInstancedStruct& CurrentStep = SkillData.Steps[CurrentStepIndex];
	for (UDiaSkillStepExecutor* Executor : StepExecutors)
	{
		if (Executor && Executor->CanExecute(CurrentStep))
		{
			FDiaSkillStepFinishedDelegate StepFinished;
			StepFinished.BindUObject(this, &ThisClass::HandleCurrentStepFinished);
			Executor->Execute(CurrentStep, this, ExecutionContext, StepFinished);
			return; 
		}
	}

	// 실행할 수 있는 Executor가 없는 경우에도 다음 스텝으로 넘어감
	HandleCurrentStepFinished();
}

void UDiaGameplayAbility::HandleCurrentStepFinished()
{
	CurrentStepIndex++;
	RunNextStep();
}

void UDiaGameplayAbility::MakeEffectSpecContextToTarget(TArray<FGameplayEffectSpecHandle>& OutContext) const
{
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(SourceASC))
		return;
	for (const auto EffectClass : SkillData.EffectsToApplyOnHit)
	{
		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();

		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), EffectContext);
		OutContext.Add(SpecHandle);
	}
}

void UDiaGameplayAbility::ActiveModifierEffect(ADiaBaseCharacter* Owner)
{
	ModifierRuntime.CDRP = 1.f;
	ModifierRuntime.MCRP = 1.f;

	if (!IsValid(Owner) || !IsValid(SkillObject))
		return;

	UDiaSkillManagerComponent* DiaSkillManagerComp = Owner->FindComponentByClass<UDiaSkillManagerComponent>();
	if (!IsValid(DiaSkillManagerComp))
		return;

	FDiaSkillVariantContext Context;
	DiaSkillManagerComp->ActiveModifierSkillUseVariants(Context, this, ModifierRuntime);
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

ADiaSkillActor* UDiaGameplayAbility::SpawnCosmeticSkillActorAtTransform(TSubclassOf<ADiaSkillActor> SkillActorClass, const FTransform& SpawnTransform, bool bAttachToAvatar)
{
	UWorld* World = GetWorld();
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!World || !SkillActorClass || !AvatarActor)
	{
		return nullptr;
	}

	FDiaSkillActorSpawnRequest SpawnRequest;
	SpawnRequest.World = World;
	SpawnRequest.SkillActorClass = SkillActorClass;
	SpawnRequest.SpawnTransform = SpawnTransform;
	SpawnRequest.OwnerActor = AvatarActor;
	SpawnRequest.Instigator = Cast<APawn>(AvatarActor);
	SpawnRequest.SkillData = &SkillData;
	SpawnRequest.SourceASC = GetAbilitySystemComponentFromActorInfo();
	SpawnRequest.DamageEffectClass = nullptr;
	SpawnRequest.OwningAbility = this;
	SpawnRequest.bLaunch = false;

	ADiaSkillActor* SpawnedActor = FDiaSkillActorSpawnHelper::SpawnSkillActor(SpawnRequest);
	if (!IsValid(SpawnedActor))
	{
		return nullptr;
	}

	SpawnedActor->SetActorEnableCollision(false);
	if (bAttachToAvatar)
	{
		SpawnedActor->AttachToActor(AvatarActor, FAttachmentTransformRules::KeepWorldTransform);
	}

	return SpawnedActor;
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
	
	AbilityMontage = nullptr;
	if (!SkillData.CastAnimation.IsNull())
	{
		AbilityMontage = SkillData.CastAnimation.LoadSynchronous();
		//로드 성공 시 로그 출력
#if WITH_EDITOR
		if (AbilityMontage)
		{
			UE_LOG(LogARPG, Log, TEXT("Loaded AbilityMontage: %s"), *AbilityMontage->GetName());
		}
		else
		{
			UE_LOG(LogARPG, Warning, TEXT("SkillData.CastAnimation is not valid"));
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

	StepExecutors.Reset();
	StepExecutors.Add(NewObject<UDiaUtilityStepExecutor>(this));
	StepExecutors.Add(NewObject<UDiaChargeStepExecutor>(this));
	StepExecutors.Add(NewObject<UDiaGroundSpawnStepExecutor>(this));
	StepExecutors.Add(NewObject<UDiaProjectileSpawnStepExecutor>(this));
	StepExecutors.Add(NewObject<UDiaMeleeSpawnStepExecutor>(this));

}

void UDiaGameplayAbility::SetSkillObject(const USkillObject* InSkillObject)
{
	SkillObject = InSkillObject;
}

TSubclassOf<ADiaSkillActor> UDiaGameplayAbility::GetSkillActorClassForSpawn() const
{
	if (const FGASUtilitySpawnData* UtilityData = SkillData.GetExtraPtr<FGASUtilitySpawnData>())
	{
		return UtilityData->SkillActorClass;
	}

	if (const FGASMeleeData* MeleeData = SkillData.GetExtraPtr<FGASMeleeData>())
	{
		return MeleeData->SkillActorClass;
	}

	if (const FGASSkillActorSpawnData* SpawnData = SkillData.GetExtraPtr<FGASSkillActorSpawnData>())
	{
		return SpawnData->SkillActorClass;
	}

	for (const FInstancedStruct& Step : SkillData.Steps)
	{
		if (const FGASProjectileSpawnStepData* ProjectileStep = Step.GetPtr<FGASProjectileSpawnStepData>())
		{
			if (ProjectileStep->ProjectileData.SkillActorClass)
			{
				return ProjectileStep->ProjectileData.SkillActorClass;
			}
		}

		if (const FGASGroundSpawnStepData* GroundStep = Step.GetPtr<FGASGroundSpawnStepData>())
		{
			if (GroundStep->GroundData.SkillActorClass)
			{
				return GroundStep->GroundData.SkillActorClass;
			}
		}

		if (const FGASMeleeSpawnStepData* MeleeStep = Step.GetPtr<FGASMeleeSpawnStepData>())
		{
			if (MeleeStep->MeleeData.SkillActorClass)
			{
				return MeleeStep->MeleeData.SkillActorClass;
			}
		}

		if (const FGASUtilityStepData* UtilityStep = Step.GetPtr<FGASUtilityStepData>())
		{
			if (UtilityStep->UtilityData.SkillActorClass)
			{
				return UtilityStep->UtilityData.SkillActorClass;
			}
		}
	}

	return nullptr;
}

void UDiaGameplayAbility::ApplySkillObjectRemovalTimer(ADiaSkillActor* SkillActor) const
{
	if (!IsValid(SkillActor))
	{
		return;
	}

	const float DurationScale = SkillActor->GetRuntimeParams().GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_DurationMultiplier, 1.f);
	const float RemainSeconds = GetSkillActorLifeSpan(SkillData) * DurationScale;
	if (RemainSeconds <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SkillActor->ArmRemovalTimer(RemainSeconds);
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

	// Set this before activation: montage failure can cancel synchronously.
	CurrentAbilityMontage = MontageToPlay;
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

	return MontageToPlay->GetPlayLength() / PlayRate;
}

void UDiaGameplayAbility::StopAbilityMontage(float BlendOutTime)
{
	UAnimMontage* MontageToStop = CurrentAbilityMontage;
	CurrentAbilityMontage = nullptr;
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
	if (ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
		if (UAnimInstance* Anim = Character->GetMesh()->GetAnimInstance(); Anim && MontageToStop)
			Anim->Montage_Stop(BlendOutTime, MontageToStop);
}

void UDiaGameplayAbility::ProcessSkillDelayEvents()
{
	if (SkillData.Steps.Num() == 0)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	ExecuteAbilityLogic(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, nullptr);
}

void UDiaGameplayAbility::OnMontageCompleted()
{
	MontageTask = nullptr;
	CurrentAbilityMontage = nullptr;

	if (!ShouldEndAbilityOnMontageCompleted() && !(bOwnsMonsterAttack && bSkillExecutionFinished))
	{
		return;
	}
	
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

bool UDiaGameplayAbility::ShouldEndAbilityOnMontageCompleted() const
{
	// Step-based skills finish when their final step completes, including delayed or blocked charges.
	return SkillData.Steps.IsEmpty() || bSkillExecutionFinished;
}

bool UDiaGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid()) return false;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	const float manaCost = CalculateFinalManaCost(ActorInfo);
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

	const float manaCost = CalculateFinalManaCost(ActorInfo);
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ManaCostEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
	if (auto* Spec = SpecHandle.Data.Get())
	{
		// 음수로 적용하여 마나 감소 (Additive이므로 -값을 더하면 감소)
		Spec->SetSetByCallerMagnitude(SkillData.ManaCostTags, -manaCost);

		Spec->DynamicGrantedTags.AddTag(SkillData.ManaCostTags);

		ASC->ApplyGameplayEffectSpecToSelf(*Spec);

		//UE_LOG(LogARPG, Log, TEXT("Applied Mana Cost: -%f (Current Mana: %f)"), manaCost, 
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
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid()) return;
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
			UE_LOG(LogARPG, Log, TEXT("Checking SetByCallerTag: %s, GetTag : %s"), *Pair.Key.ToString(), *FDiaGameplayTags::Get().CoolDown_GetTag(SkillData.SkillID).ToString());
			if (Pair.Key.MatchesTagExact(FDiaGameplayTags::Get().CoolDown_GetTag(SkillData.SkillID)))
			{
				CooldownTag = Pair.Key;
				cooldownDuration = Pair.Value; // Spec에 저장된 Duration 사용
				break;
			}
		}
	}
	
	UE_LOG(LogARPG, Log, TEXT("[ApplyCooldown] Found CooldownTag: %s, Base Duration: %.2f"), *CooldownTag.ToString(), cooldownDuration);

	cooldownDuration = CalculateFinalCooldown(ActorInfo, cooldownDuration); // Duration이 음수인 경우 0으로 보정

	UE_LOG(LogARPG, Log, TEXT("[ApplyCooldown] SkillID: %d, CooldownTag: %s, Duration: %.2f"), 
		AbilitySpec->InputID, *CooldownTag.ToString(), cooldownDuration);
	if (!CooldownTag.IsValid())
	{
		UE_LOG(LogARPG, Error, TEXT("[ApplyCooldown] No cooldown tag found in Spec for SkillID: %d"), AbilitySpec->InputID);
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
		
		//UE_LOG(LogARPG, Log, TEXT("[ApplyCooldown] Applied cooldown for SkillID: %d, Duration: %.2f, Tag: %s"), 
		//	AbilitySpec->InputID, cooldownDuration, *CooldownTag.ToString());
	}
}
