#include "GAS/Abilities/DiaMeleeAbility.h"
#include "GAS/DiaAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "GameFramework/Character.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Character/DiaCharacter.h"
#include "DiaBaseCharacter.h"

#include "Components/AudioComponent.h"
#include <AbilitySystemBlueprintLibrary.h>

UDiaMeleeAbility::UDiaMeleeAbility()
{
	// Set default values
	AttackRange = 200.0f;
	AttackAngle = 60.0f;
	AttackOffset = FVector(100.0f, 0.0f, 0.0f);
	
	CurrentHitCount = 0;
	TotalHitCount = 1;
	HitInterval = 0.0f;

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
		UE_LOG(LogTemp, Warning, TEXT("DiaMeleeAbility: Invalid ActorInfo or AvatarActor"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	HitActors.Empty();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// HitCount 체크: 1이면 단일 히트, 2 이상이면 Multi Hit
	TotalHitCount = FMath::Max(1, SkillData.HitCount);
	HitInterval = FMath::Max(0.0f, SkillData.HitInterval);

	if (TotalHitCount == 1)
	{
		PerformHitDetection();
	}
	else
	{
		StartMultiHit();
	}
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
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!ActorInfo.AvatarActor.IsValid())
	{
		return;
	}

	ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(ActorInfo.AvatarActor.Get());
	if (!Character)
	{
		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World)
	{
		return;
	}

	FVector CharacterLocation = Character->GetActorLocation();
	FRotator CharacterRotation = Character->GetActorRotation();

	FVector AttackCenter = CharacterLocation + CharacterRotation.RotateVector(AttackOffset);

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
	QueryParams.bReturnPhysicalMaterial = false;

	bool bHit = World->SweepMultiByChannel(
		HitResults,
		AttackCenter,
		AttackCenter,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRange),
		QueryParams
	);
	
	if (bShowDebugShape)
	{
		DrawDebugSphere(World, AttackCenter, AttackRange, 12, FColor::Red, false, 2.0f);
		
		FVector ForwardVector = CharacterRotation.Vector();
		float HalfAngleRad = FMath::DegreesToRadians(AttackAngle * 0.5f);
		
		for (int32 i = 0; i <= 8; ++i)
		{
			float CurrentAngle = -HalfAngleRad + (HalfAngleRad * 2.0f * i / 8.0f);
			FVector ConeDirection = ForwardVector.RotateAngleAxis(FMath::RadiansToDegrees(CurrentAngle), FVector::UpVector);
			DrawDebugLine(World, AttackCenter, AttackCenter + ConeDirection * AttackRange, FColor::Yellow, false, 2.0f);
		}
	}

	if (bHit)
	{
		FVector ForwardVector = CharacterRotation.Vector();
		float HalfAngleRad = FMath::DegreesToRadians(AttackAngle * 0.5f);

		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!HitActor || HitActors.Contains(HitActor))
			{
				continue;
			}

			//actor 0번 tag는 character
			//1번은 player or monster
			//같은 타입은 피격에서 제외
			TArray<FName> OwnerTags = Character->Tags;
			if (HitActor->ActorHasTag(OwnerTags[1]))
			{
				continue;
			}
			
			Character->SetTargetActor(Cast<ADiaBaseCharacter>(HitActor));

			FVector ToTarget = (HitActor->GetActorLocation() - AttackCenter).GetSafeNormal();
			float DotProduct = FVector::DotProduct(ForwardVector, ToTarget);
			float AngleToTarget = FMath::Acos(DotProduct);

			if (AngleToTarget <= HalfAngleRad)
			{
				UE_LOG(LogTemp, Log, TEXT("DiaMeleeAbility: Hit Actor: %s"), *HitActor->GetName());
				HitActors.Add(HitActor);

				ApplyDamageToTarget(HitActor);

				SpawnHitEffectAtLocation(HitActor->GetActorLocation());

				PlayHitSoundAtLocation(HitActor->GetActorLocation());
			}
		}
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

	UE_LOG(LogTemp, Warning, TEXT("Target Class: %s"), *Target->GetClass()->GetName());
    
    // ADiaBaseCharacter인지 확인
    ADiaBaseCharacter* DiaChar = Cast<ADiaBaseCharacter>(Target);
    UE_LOG(LogTemp, Warning, TEXT("Cast to ADiaBaseCharacter: %s"), DiaChar ? TEXT("Success") : TEXT("Failed"));

	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	// GAS 경로: Execution 기반 대미지 적용
	const UDiaAttributeSet* MyAttr = GetAbilitySystemComponentFromActorInfo()->GetSet<UDiaAttributeSet>();
	const float AttackPower = MyAttr ? MyAttr->GetAttackPower() : 0.f;
	const float BaseDamage = SkillData.BaseDamage + AttackPower;

	UE_LOG(LogTemp, Log, TEXT("DiaMeleeAbility: Applying Damage - BaseDamage: %.2f"), BaseDamage);
	ApplyDamageToASC(TargetASC, BaseDamage , 1.0f);
}

void UDiaMeleeAbility::StartMultiHit()
{
	CurrentHitCount = 0;

	UE_LOG(LogTemp, Log, TEXT("DiaMeleeAbility: Starting Multi Hit - TotalHits: %d, Interval: %.2f"), TotalHitCount, HitInterval);

	ProcessNextHit();
}

void UDiaMeleeAbility::ProcessNextHit()
{
	++CurrentHitCount;

	// 모든 히트 완료 시 종료
	if (CurrentHitCount > TotalHitCount)
	{
		UE_LOG(LogTemp, Log, TEXT("DiaMeleeAbility: All hits completed"));
		return;
	}

	// 다음 히트 스케줄링
	UWorld* World = GetWorld();
	if (World && HitInterval > 0.0f)
	{
		World->GetTimerManager().SetTimer(
			MultiHitTimerHandle,
			this,
			&UDiaMeleeAbility::PerformHitDetection,
			HitInterval,
			false
		);
	}
	else
	{
		// 간격이 0이면 즉시 실행
		PerformHitDetection();
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
