#include "GAS/Abilities/DiaMeleeAbility.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaMeleeTargeting.h"
#include "AbilitySystemComponent.h"

#include "GameFramework/Character.h"

#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Character/DiaCharacter.h"
#include "DiaBaseCharacter.h"

#include "Components/AudioComponent.h"
#include <AbilitySystemBlueprintLibrary.h>
#include "Logging/ARPGLogChannels.h"

UDiaMeleeAbility::UDiaMeleeAbility()
{
	// Set default values
	AttackRange = 200.0f;
	AttackAngle = 60.0f;
	AttackOffset = FVector(100.0f, 0.0f, 0.0f);
	
	CurrentHitCount = 0;
	TotalHitCount = 1;
	HitInterval = 0.0f;
}

void UDiaMeleeAbility::InitializeWithSkillData(const FGASSkillData& InSkillData)
{
	Super::InitializeWithSkillData(InSkillData);

	const FGASMeleeData* M = InSkillData.GetExtraPtr<FGASMeleeData>();
	if (!M)
	{
		return;
	}

	AttackRange = M->AttackRange;
	AttackAngle = M->AttackAngle;
	AttackOffset = M->AttackOffset;
	bShowDebugShape = M->bShowDebugShape;

	// Set ability tags
	//FGameplayTagContainer Tags;
	//Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Melee")));
	//SetAssetTags(Tags);
	//
	//FGameplayTagContainer BlockTags;
	//BlockTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));
	//BlockAbilitiesWithTag = BlockTags;
}

void UDiaMeleeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		ARPG_SKILL_LOG(Warning, TEXT("Invalid ActorInfo or AvatarActor"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	HitActors.Empty();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// HitCount 체크: 1이면 단일 히트, 2 이상이면 Multi Hit
	TotalHitCount = FMath::Max(1, SkillData.HitCount);
	HitInterval = FMath::Max(0.0f, SkillData.HitInterval);

}

void UDiaMeleeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Multi Hit 타이머 정리
	ClearMultiHitTimer();
	
	// 카운트 초기화
	CurrentHitCount = 0;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDiaMeleeAbility::PerformHitDetection()
{
	ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(GetAvatarActorFromActorInfo());
	if (!IsActive() || !Character) return;
	FGASMeleeData Data;
	Data.AttackRange = AttackRange;
	Data.AttackAngle = AttackAngle;
	Data.AttackOffset = AttackOffset;
	Data.bShowDebugShape = bShowDebugShape;
	TArray<ADiaBaseCharacter*> Targets;
	DiaMeleeTargeting::GatherTargets(*Character, Data, Character->GetActorLocation(), Character->GetActorRotation(), Targets);
	for (ADiaBaseCharacter* Target : Targets)
	{
		if (HitActors.Contains(Target)) continue;
		HitActors.Add(Target);
		ApplyDamageToTarget(Target);
		SpawnHitEffectAtLocation(Target->GetActorLocation());
		PlayHitSoundAtLocation(Target->GetActorLocation());
	}
}

void UDiaMeleeAbility::OnMeleeHitFrame()
{
	PerformHitDetection();
}

void UDiaMeleeAbility::ApplyDamageToTarget(AActor* Target)
{
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!Target || !ActorInfo.AbilitySystemComponent.IsValid())
	{
		return;
	}

	ARPG_SKILL_VVLOG(TEXT("Target Class=%s"), *Target->GetClass()->GetName());
    
    // ADiaBaseCharacter인지 확인
    ADiaBaseCharacter* DiaChar = Cast<ADiaBaseCharacter>(Target);
    ARPG_SKILL_VVLOG(TEXT("Cast to ADiaBaseCharacter=%s"), DiaChar ? TEXT("Success") : TEXT("Failed"));

	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	ApplyDamageToASC(TargetASC, SkillData.BaseDamage , 1.0f);
}

void UDiaMeleeAbility::StartMultiHit()
{
	CurrentHitCount = 0;

	ARPG_SKILL_VLOG(TEXT("Starting Multi Hit. TotalHits=%d, Interval=%.2f"), TotalHitCount, HitInterval);

	ProcessNextHit();
}

void UDiaMeleeAbility::ProcessSkillDelayEvents()
{
	if (SkillData.Steps.Num() > 0)
	{
		Super::ProcessSkillDelayEvents();
		return;
	}

	// HitCount 체크: 1이면 단일 히트, 2 이상이면 Multi Hit
	TotalHitCount = FMath::Max(1, SkillData.HitCount);
	HitInterval = FMath::Max(0.0f, SkillData.HitInterval);

	if (TotalHitCount == 1)
	{
		HitActors.Empty();
		PerformHitDetection();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		StartMultiHit();
	}
}

void UDiaMeleeAbility::ProcessNextHit()
{
	++CurrentHitCount;

	// 모든 히트 완료 시 종료
	if (CurrentHitCount > TotalHitCount)
	{
		ARPG_SKILL_VLOG(TEXT("All hits completed"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// 다음 히트 스케줄링
	HitActors.Empty();
	PerformHitDetection();

	if (CurrentHitCount >= TotalHitCount)
	{
		ARPG_SKILL_VLOG(TEXT("All hits completed"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	UWorld* World = GetWorld();
	if (World && HitInterval > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			MultiHitTimerHandle,
			this,
			&UDiaMeleeAbility::ProcessNextHit,
			HitInterval,
			false
		);
	}
	else
	{
		// 간격이 0이면 즉시 실행
		ProcessNextHit();
	}
}

void UDiaMeleeAbility::ClearMultiHitTimer()
{
	UWorld* World = GetWorld();
	if (World && MultiHitTimerHandle.IsValid())
	{
		World->GetTimerManager().ClearTimer(MultiHitTimerHandle);
		MultiHitTimerHandle.Invalidate();
	}
}
