// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/DiaChargeAbility.h"
#include "GAS/DiaGameplayTags.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "GAS/DiaAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include <AbilitySystemBlueprintLibrary.h>

UDiaChargeAbility::UDiaChargeAbility()
{
	// 통합 태그 관리 시스템 사용
	ActivationOwnedTags.AddTag(FDiaGameplayTags::Get().State_Charging);
}

void UDiaChargeAbility::InitializeWithSkillData(const FGASSkillData& InSkillData)
{
	Super::InitializeWithSkillData(InSkillData);

	const FGASChargeData* C = InSkillData.GetExtraPtr<FGASChargeData>();
	if (!C)
	{
		return;
	}

	ChargeDelay = C->ChargeDelay;
	ChargeDuration = C->ChargeDuration;
	UpdateTime = C->UpdateTime;
	ChargeDist = C->ChargeDist;
	if (C->PathOffsetCurve)
	{
		PathOffsetCurve = C->PathOffsetCurve;
	}
}

void UDiaChargeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!ActorInfo)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EndLoc = CalcSweepPosition(ActorInfo);

	UAbilityTask_ApplyRootMotionMoveToForce* ForceRootMotionMoveTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
		this, 
		FName(TEXT("ChargeMoveTask")),
		EndLoc, 
		ChargeDuration,
		true, 
		EMovementMode::MOVE_Flying,
		true, 
		PathOffsetCurve,
		ERootMotionFinishVelocityMode::SetVelocity,
		FVector::ZeroVector, 
		0.f);

	if (ForceRootMotionMoveTask)
	{
		ForceRootMotionMoveTask->OnTimedOutAndDestinationReached.AddDynamic(this, &UDiaChargeAbility::OnDashFinished);

		ForceRootMotionMoveTask->ReadyForActivation();

	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}

}

void UDiaChargeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FVector UDiaChargeAbility::CalcSweepPosition(const FGameplayAbilityActorInfo* ActorInfo)
{

	float CapsuleHalfHeight = 0.f;
	float CapsuleRadius = 0.f;
	if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
		{
			CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
			CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
		}
	}

	//charge 거리 계산
	StartLoc = ActorInfo->AvatarActor->GetActorLocation();
	FVector ForwardVector = ActorInfo->AvatarActor->GetActorForwardVector();
	FVector DistLoc = StartLoc + ForwardVector * ChargeDist;

	// Implement your logic to determine the sweep direction
	FHitResult HitResults;
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(
		CapsuleRadius,
		CapsuleHalfHeight / 2.f
	);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ActorInfo->AvatarActor.Get());  // 자기 자신 무시

	bool bIsSweep = GetWorld()->SweepSingleByChannel(
		HitResults,
		StartLoc,
		DistLoc,
		FQuat::Identity,
		ECC_WorldStatic,
		CapsuleShape,
		QueryParams  // 추가
	);

	if (bIsSweep)
	{
		return HitResults.ImpactPoint;
	}
	else
	{
		return DistLoc;
	}
}

void UDiaChargeAbility::ApplyHitToActorsInPath(AActor* TargetActor)
{
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!TargetActor || !ActorInfo.AbilitySystemComponent.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC)
	{
		return;
	}

	const UDiaAttributeSet* MyAttr = GetAbilitySystemComponentFromActorInfo()->GetSet<UDiaAttributeSet>();
	const float AttackPower = MyAttr ? MyAttr->GetAttackPower() : 0.f;
	const float BaseDamage = SkillData.BaseDamage + AttackPower;
	ApplyDamageToASC(TargetASC, BaseDamage, 1.0f);
}

void UDiaChargeAbility::OnDashFinished()
{
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CurrentActorInfo->AvatarActor.Get());

	// 캡슐 컴포넌트 크기 가져오기
	FVector BoxExtent = FVector(50.f, 50.f, 88.f);
	if (ACharacter* Character = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get()))
	{
		if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
		{
			float Radius = CapsuleComp->GetScaledCapsuleRadius();
			float HalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
			BoxExtent = FVector(Radius, Radius, HalfHeight);
		}
	}

	bool bIsHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLoc,
		EndLoc,
		FQuat::Identity,
		ECollisionChannel::ECC_EngineTraceChannel2,
		FCollisionShape::MakeBox(BoxExtent),
		QueryParams
	);

	if (bIsHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				UE_LOG(LogTemp, Log, TEXT("[ChargeAbility] Hit: %s at %s"),
					*HitActor->GetName(), *Hit.ImpactPoint.ToString());

				ApplyHitToActorsInPath(HitActor);
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
