// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Executor/DiaSkillStepExecutor.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGASHelper.h"
#include "GAS/DiaGameplayTags.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaBaseCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GAS/DiaAttributeSet.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"

#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Skill/DiaSkillActor.h"
#include "Logging/ARPGLogChannels.h"

namespace
{
float GetStepSkillLevelScale(const UDiaGameplayAbility& Ability)
{
	return 1.f + FMath::Max(0, Ability.GetAbilityLevel() - 1) * 0.25f;
}

float GetUtilityDuration(const FGASSkillData& SkillData, float DefaultDuration)
{
	return SkillData.HitInterval > 0.f ? SkillData.HitInterval : DefaultDuration;
}

void ApplyTemporaryAttributeModifier(UWorld* World, UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute, float Delta, float Duration)
{
	if (!World || !ASC || FMath::IsNearlyZero(Delta))
	{
		return;
	}

	ASC->ApplyModToAttribute(Attribute, EGameplayModOp::Additive, Delta);

	if (Duration <= 0.f)
	{
		return;
	}

	TWeakObjectPtr<UAbilitySystemComponent> WeakASC = ASC;
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateLambda([WeakASC, Attribute, Delta]()
		{
			if (UAbilitySystemComponent* PinnedASC = WeakASC.Get())
			{
				PinnedASC->ApplyModToAttribute(Attribute, EGameplayModOp::Additive, -Delta);
			}
		}),
		Duration,
		false);
}

FTransform MakeUtilitySkillActorTransform(const AActor& AvatarActor, const FGASUtilitySpawnData& UtilityData, const FVector& BaseLocation)
{
	const FRotator SpawnRotation = UtilityData.bUseOwnerRotation ? AvatarActor.GetActorRotation() : FRotator::ZeroRotator;
	return FTransform(SpawnRotation, BaseLocation + SpawnRotation.RotateVector(UtilityData.SpawnOffset));
}

void SpawnUtilityActor(UDiaGameplayAbility& Ability, AActor& AvatarActor, const FGASUtilitySpawnData& UtilityData, TSubclassOf<ADiaSkillActor> SkillActorClass, const FVector& BaseLocation)
{
	if (!SkillActorClass)
	{
		return;
	}

	const FTransform SpawnTransform = MakeUtilitySkillActorTransform(AvatarActor, UtilityData, BaseLocation);
	Ability.SpawnCosmeticSkillActorAtTransform(SkillActorClass, SpawnTransform, UtilityData.bAttachToOwner);
}
}

bool UDiaChargeStepExecutor::CanExecute(const FInstancedStruct& StepData) const
{
    if(StepData.GetScriptStruct() == FGASChargeStepData::StaticStruct())
    {
        return true;
	}
    return false;
}

void UDiaChargeStepExecutor::Execute(const FInstancedStruct& StepData, UDiaGameplayAbility* Ability, FDiaSkillExecutionContext& Context, FDiaSkillStepFinishedDelegate OnFinished)
{
	const FGASChargeStepData* CharStep = StepData.GetPtr<FGASChargeStepData>();
	if (!CharStep || !Ability)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	const FGASChargeData& ChargeData = CharStep->ChargeData;

	CachedAbility = Ability;
	const FGameplayAbilityActorInfo& CurrentActorInfo = CachedAbility->GetActorInfo();

	CacheOnFinished = OnFinished;
	CachedChargeDistance = FMath::Max(0.f, ChargeData.ChargeDist);


	EndLoc = CalcSweepPosition(CurrentActorInfo);

	UAbilityTask_ApplyRootMotionMoveToForce* ForceRootMotionMoveTask = UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(
		Ability,
		FName(TEXT("ChargeMoveTask")),
		EndLoc,
		ChargeData.ChargeDuration,
		true,
		EMovementMode::MOVE_Flying,
		true,
		ChargeData.PathOffsetCurve,
		ERootMotionFinishVelocityMode::SetVelocity,
		FVector::ZeroVector,
		0.f);

	if (ForceRootMotionMoveTask)
	{
		ForceRootMotionMoveTask->OnTimedOutAndDestinationReached.AddDynamic(this, &ThisClass::OnDashFinished);

		ForceRootMotionMoveTask->ReadyForActivation();
	}
	else
	{
		CacheOnFinished.ExecuteIfBound();
	}
	Context.LastStepLocation = EndLoc;
}

FVector UDiaChargeStepExecutor::CalcSweepPosition(const FGameplayAbilityActorInfo& ActorInfo)
{

	float CapsuleHalfHeight = 0.f;
	float CapsuleRadius = 0.f;
	if (ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get()))
	{
		if (UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent())
		{
			CapsuleHalfHeight = CapsuleComp->GetScaledCapsuleHalfHeight();
			CapsuleRadius = CapsuleComp->GetScaledCapsuleRadius();
		}
	}

	//charge 거리 계산
	StartLoc = ActorInfo.AvatarActor->GetActorLocation();
	FVector ForwardVector = ActorInfo.AvatarActor->GetActorForwardVector();
	FVector DistLoc = StartLoc + ForwardVector * CachedChargeDistance;

	// Implement your logic to determine the sweep direction
	FHitResult HitResults;
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(
		CapsuleRadius,
		CapsuleHalfHeight / 2.f
	);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ActorInfo.AvatarActor.Get());  // 자기 자신 무시

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

void UDiaChargeStepExecutor::OnDashFinished()
{
	const FGameplayAbilityActorInfo& CurrentActorInfo = CachedAbility->GetActorInfo();

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CurrentActorInfo.AvatarActor.Get());

	// 캡슐 컴포넌트 크기 가져오기
	FVector BoxExtent = FVector(50.f, 50.f, 88.f);
	if (ACharacter* Character = Cast<ACharacter>(CurrentActorInfo.AvatarActor.Get()))
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
				UE_LOG(LogARPG, Log, TEXT("[ChargeAbility] Hit: %s at %s"),
					*HitActor->GetName(), *Hit.ImpactPoint.ToString());

				//ApplyHitToActorsInPath(HitActor);
			}
		}
	}

	if(CacheOnFinished.IsBound())
	{
		CacheOnFinished.ExecuteIfBound();
		CacheOnFinished.Unbind();
	}
}


bool UDiaGroundSpawnStepExecutor::CanExecute(const FInstancedStruct& StepData) const
{
    if (StepData.GetScriptStruct() == FGASGroundSpawnStepData::StaticStruct())
    {
        return true;
    }

    return false;
}

void UDiaGroundSpawnStepExecutor::Execute(const FInstancedStruct& StepData, UDiaGameplayAbility* Ability, FDiaSkillExecutionContext& Context, FDiaSkillStepFinishedDelegate OnFinished)
{
	const FGASGroundSpawnStepData* GroundStep = StepData.GetPtr<FGASGroundSpawnStepData>();
	if (!GroundStep || !Ability)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	const FGASGroundData& GroundData = GroundStep->GroundData;
	if (!GroundData.SkillActorClass)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(Ability->GetAvatarActorFromActorInfo());
	if (!Character)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
	const bool bIsPlayer = Character->ActorHasTag(FDiaGameplayTags::Get().Actor_Player.GetTagName());

    //HACK: 플레이어가 아니면 캐릭터 방향으로 발사하도록 강제
	const bool bUseOwnerRotation = !bIsPlayer || GroundData.bUseOwnerRotation;

	FVector SpawnLocation = bIsPlayer ? UDiaGASHelper::GetMouseWorldLocation(ActorInfo) : Character->GetActorLocation();
	if (GroundStep->bUseLastStepEndLocation && !Context.LastStepLocation.IsNearlyZero())
	{
		SpawnLocation = Context.LastStepLocation;
	}

	const FRotator SpawnRotation = bUseOwnerRotation ? Character->GetActorRotation() : FRotator::ZeroRotator;

    FGameplayAbilityTargetDataHandle TargetDataHandle;

    FGameplayAbilityTargetData_LocationInfo LocationData;
    LocationData.SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationData.SourceLocation.LiteralTransform = FTransform(SpawnRotation, SpawnLocation);
    LocationData.TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	LocationData.TargetLocation.LiteralTransform = FTransform(SpawnRotation, SpawnLocation);
    TargetDataHandle.Add(new FGameplayAbilityTargetData_LocationInfo(LocationData));

    FDiaSkillVariantContext VariantContext;
	VariantContext.SkillActorClass = GroundData.SkillActorClass;
    VariantContext.TargetData = TargetDataHandle;

    UDiaSkillManagerComponent* DiaSkillManagerComp = Character->FindComponentByClass<UDiaSkillManagerComponent>();
	if (DiaSkillManagerComp)
	{
		CachedExecutionContext = &Context;
		CachedVariantContext = VariantContext;
		CachedOnFinished = OnFinished;

		FDiaSkillSpawnFinishedDelegate SpawnFinished;
		SpawnFinished.BindUObject(this, &ThisClass::OnVariantSpawnFinished);
		DiaSkillManagerComp->SpawnSkillActorUseVariants(CachedVariantContext, Ability, SpawnFinished);
		return;
	}

	OnFinished.ExecuteIfBound();
}

void UDiaGroundSpawnStepExecutor::OnVariantSpawnFinished()
{
	if (CachedExecutionContext)
	{
		for (ADiaSkillActor* SpawnedActor : CachedVariantContext.SkillActors)
		{
			if (SpawnedActor)
			{
				CachedExecutionContext->SpawnedActors.Add(SpawnedActor);
				CachedExecutionContext->LastStepLocation = SpawnedActor->GetActorLocation();
			}
		}
	}

	CachedExecutionContext = nullptr;
	CachedVariantContext = FDiaSkillVariantContext();
	FDiaSkillStepFinishedDelegate Finished = CachedOnFinished;
	CachedOnFinished.Unbind();
	Finished.ExecuteIfBound();
}

bool UDiaMeleeSpawnStepExecutor::CanExecute(const FInstancedStruct& StepData) const
{
	if (StepData.GetScriptStruct() == FGASMeleeSpawnStepData::StaticStruct())
	{
		return true;
	}

	return false;
}

void UDiaMeleeSpawnStepExecutor::Execute(const FInstancedStruct& StepData, UDiaGameplayAbility* Ability, FDiaSkillExecutionContext& Context, FDiaSkillStepFinishedDelegate OnFinished)
{
	const FGASMeleeSpawnStepData* MeleeStep = StepData.GetPtr<FGASMeleeSpawnStepData>();
	if (!MeleeStep || !Ability)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(Ability->GetAvatarActorFromActorInfo());
	if (!Character)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	const FGASMeleeData& MeleeData = MeleeStep->MeleeData;
	const FVector CharacterLocation = Character->GetActorLocation();
	const FRotator CharacterRotation = Character->GetActorRotation();
	const FVector AttackCenter = CharacterLocation + CharacterRotation.RotateVector(MeleeData.AttackOffset);

	if (MeleeData.SkillActorClass)
	{
		Ability->SpawnCosmeticSkillActorAtTransform(MeleeData.SkillActorClass, FTransform(CharacterRotation, AttackCenter));
	}

	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
	QueryParams.bReturnPhysicalMaterial = false;

	const bool bHit = World->SweepMultiByChannel(
		HitResults,
		AttackCenter,
		AttackCenter,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(MeleeData.AttackRange),
		QueryParams);

	if (MeleeData.bShowDebugShape)
	{
		DrawDebugSphere(World, AttackCenter, MeleeData.AttackRange, 12, FColor::Red, false, 2.0f);

		const FVector ForwardVector = CharacterRotation.Vector();
		const float HalfAngleRad = FMath::DegreesToRadians(MeleeData.AttackAngle * 0.5f);
		for (int32 i = 0; i <= 8; ++i)
		{
			const float CurrentAngle = -HalfAngleRad + (HalfAngleRad * 2.0f * i / 8.0f);
			const FVector ConeDirection = ForwardVector.RotateAngleAxis(FMath::RadiansToDegrees(CurrentAngle), FVector::UpVector);
			DrawDebugLine(World, AttackCenter, AttackCenter + ConeDirection * MeleeData.AttackRange, FColor::Yellow, false, 2.0f);
		}
	}

	if (bHit)
	{
		TSet<AActor*> HitActors;
		const FVector ForwardVector = CharacterRotation.Vector();
		const float HalfAngleRad = FMath::DegreesToRadians(MeleeData.AttackAngle * 0.5f);

		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (!IsValid(HitActor) || HitActors.Contains(HitActor))
			{
				continue;
			}

			bool bSameSide = false;
			static const FName CharacterTag = FName(TEXT("Character"));
			for (const FName& OwnerTag : Character->Tags)
			{
				if (OwnerTag == CharacterTag)
				{
					continue;
				}

				if (HitActor->ActorHasTag(OwnerTag))
				{
					bSameSide = true;
					break;
				}
			}

			if (bSameSide)
			{
				continue;
			}

			const FVector ToTarget = (HitActor->GetActorLocation() - AttackCenter).GetSafeNormal();
			const float DotProduct = FMath::Clamp(FVector::DotProduct(ForwardVector, ToTarget), -1.f, 1.f);
			const float AngleToTarget = FMath::Acos(DotProduct);
			if (AngleToTarget > HalfAngleRad)
			{
				continue;
			}

			HitActors.Add(HitActor);
			Character->SetTargetActor(Cast<ADiaBaseCharacter>(HitActor));
			Ability->ApplySkillDamageToActor(HitActor);
		}
	}

	Context.LastStepLocation = AttackCenter;
	OnFinished.ExecuteIfBound();
}

bool UDiaUtilityStepExecutor::CanExecute(const FInstancedStruct& StepData) const
{
	return StepData.GetScriptStruct() == FGASUtilityStepData::StaticStruct();
}

void UDiaUtilityStepExecutor::Execute(const FInstancedStruct& StepData, UDiaGameplayAbility* Ability, FDiaSkillExecutionContext& Context, FDiaSkillStepFinishedDelegate OnFinished)
{
	const FGASUtilityStepData* UtilityStep = StepData.GetPtr<FGASUtilityStepData>();
	if (!UtilityStep || !Ability)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	AActor* AvatarActor = Ability->GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = nullptr;
	if (ADiaBaseCharacter* BaseCharacter = Cast<ADiaBaseCharacter>(AvatarActor))
	{
		ASC = BaseCharacter->GetAbilitySystemComponent();
	}

	if (!ASC || !AvatarActor)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	const FGASSkillData& SkillData = Ability->GetSkillData();
	const FGASUtilitySpawnData& UtilityData = UtilityStep->UtilityData;
	const float LevelScale = GetStepSkillLevelScale(*Ability);

	switch (SkillData.SkillType)
	{
	case EGASSkillType::Heal:
	{
		const float HealAmount = FMath::Abs(SkillData.BaseDamage) * LevelScale;
		if (HealAmount > 0.f)
		{
			ASC->ApplyModToAttribute(UDiaAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, HealAmount);
			SpawnUtilityActor(*Ability, *AvatarActor, UtilityData, UtilityData.SkillActorClass, AvatarActor->GetActorLocation());
		}
		break;
	}
	case EGASSkillType::Movement:
	{
		if (SkillData.TargetType != EGASTargetType::Ground || !Ability->GetActorInfo().PlayerController.IsValid())
		{
			break;
		}

		const FVector MouseWorldLocation = UDiaGASHelper::GetMouseWorldLocation(Ability->GetActorInfo());
		if (MouseWorldLocation.ContainsNaN())
		{
			break;
		}

		const FVector StartLocation = AvatarActor->GetActorLocation();
		FVector Delta = MouseWorldLocation - StartLocation;
		Delta.Z = 0.f;

		const float MaxDistance = SkillData.BaseDamage > 0.f ? SkillData.BaseDamage : 600.f;
		const FVector ClampedDelta = Delta.GetClampedToMaxSize(MaxDistance);
		FVector Destination = StartLocation + ClampedDelta;
		Destination.Z = StartLocation.Z;

		SpawnUtilityActor(*Ability, *AvatarActor, UtilityData, UtilityData.SkillActorClass, StartLocation);

		const bool bTeleported = AvatarActor->TeleportTo(Destination, AvatarActor->GetActorRotation(), false, true);
		if (bTeleported)
		{
			SpawnUtilityActor(*Ability, *AvatarActor, UtilityData, UtilityData.SecondarySkillActorClass, Destination);
		}
		break;
	}
	case EGASSkillType::Buff:
	{
		UWorld* World = Ability->GetWorld();
		if (!World)
		{
			break;
		}

		if (SkillData.SkillID == 1005)
		{
			const float Duration = GetUtilityDuration(SkillData, 3.f);
			const float SpeedBonus = FMath::Max(SkillData.BaseDamage, 250.f) * LevelScale;
			ApplyTemporaryAttributeModifier(World, ASC, UDiaAttributeSet::GetMovementSpeedAttribute(), SpeedBonus, Duration);
			SpawnUtilityActor(*Ability, *AvatarActor, UtilityData, UtilityData.SkillActorClass, AvatarActor->GetActorLocation());
			break;
		}

		if (SkillData.SkillID == 1010)
		{
			const float Duration = GetUtilityDuration(SkillData, 8.f);
			const float AttackPowerBonus = FMath::Max(SkillData.BaseDamage, 30.f) * LevelScale;
			ApplyTemporaryAttributeModifier(World, ASC, UDiaAttributeSet::GetAttackPowerAttribute(), AttackPowerBonus, Duration);

			const float DefenseMultiplier = FMath::Clamp(SkillData.DamageMultiplierPerHit, 0.f, 1.f);
			if (DefenseMultiplier < 1.f)
			{
				const float CurrentDefense = ASC->GetNumericAttribute(UDiaAttributeSet::GetDefenseAttribute());
				const float DefensePenalty = -CurrentDefense * (1.f - DefenseMultiplier);
				ApplyTemporaryAttributeModifier(World, ASC, UDiaAttributeSet::GetDefenseAttribute(), DefensePenalty, Duration);
			}

			SpawnUtilityActor(*Ability, *AvatarActor, UtilityData, UtilityData.SkillActorClass, AvatarActor->GetActorLocation());
		}
		break;
	}
	default:
		break;
	}

	Context.LastStepLocation = AvatarActor->GetActorLocation();

	OnFinished.ExecuteIfBound();
}

bool UDiaProjectileSpawnStepExecutor::CanExecute(const FInstancedStruct& StepData) const
{
	if (StepData.GetScriptStruct() == FGASProjectileSpawnStepData::StaticStruct())
	{
		return true;
	}

	return false;
}

void UDiaProjectileSpawnStepExecutor::Execute(const FInstancedStruct& StepData, UDiaGameplayAbility* Ability, FDiaSkillExecutionContext& Context, FDiaSkillStepFinishedDelegate OnFinished)
{
	if (!Ability)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	const FGASProjectileSpawnStepData* ProjectileStep = StepData.GetPtr<FGASProjectileSpawnStepData>();
	if (!ProjectileStep)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
	const FGASProjectileData* ProjectileData = &ProjectileStep->ProjectileData;

	if (!ActorInfo.AvatarActor.IsValid() || !ProjectileData->SkillActorClass)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get());
	if (!Character)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World)
	{
		OnFinished.ExecuteIfBound();
		return;
	}

	// Calculate launch direction (수평 방향으로 정규화 + 최소 거리 보정)
	FVector LaunchDirection = CalcSpawnLocation(ActorInfo, ProjectileData);
	if (LaunchDirection.IsNearlyZero())
	{
		// 마우스 위치가 애매하거나 계산에 실패하면 캐릭터 전방으로 발사
		LaunchDirection = Character->GetActorForwardVector();
	}
	LaunchDirection.Z = 0.0f;
	LaunchDirection.Normalize();

	// Calculate spawn location: 발사 방향으로 일정 거리 앞
	FVector CharacterLocation = Character->GetActorLocation();
	const float SpawnDistance = FMath::Max(ProjectileData->ProjectileOffset.Size(), ProjectileData->MinimumRange);
	FVector SpawnLocation = CharacterLocation + LaunchDirection * SpawnDistance;

#if UE_EDITOR
	DrawDebugLine(World, CharacterLocation, SpawnLocation, FColor::Green, false, 1.5f, 0, 2.0f);
#endif


	if (IsValid(ProjectileData->SkillActorClass))
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle;

		FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
		LocationData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocationData->SourceLocation.LiteralTransform = FTransform(LaunchDirection.Rotation(), SpawnLocation);
		LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocationData->TargetLocation.LiteralTransform = FTransform(LaunchDirection.Rotation(), SpawnLocation + LaunchDirection * 100.0f); // 약간 앞쪽
		TargetDataHandle.Add(LocationData);

		FDiaSkillVariantContext VariantContext;
		VariantContext.SkillActorClass = ProjectileData->SkillActorClass;
		VariantContext.TargetData = TargetDataHandle;
		VariantContext.BaseSpawnCount = ProjectileData->bFireMultipleProjectiles ? FMath::Max(1, ProjectileData->ProjectileCount) : 1;
		VariantContext.SpreadAngle = ProjectileData->SpreadAngle;

		UDiaSkillManagerComponent* DiaSkillManagerComp = Character->FindComponentByClass<UDiaSkillManagerComponent>();
		if (!DiaSkillManagerComp)
		{
			OnFinished.ExecuteIfBound();
			return;
		}

		CachedExecutionContext = &Context;
		CachedVariantContext = VariantContext;
		CachedOnFinished = OnFinished;

		FDiaSkillSpawnFinishedDelegate SpawnFinished;
		SpawnFinished.BindUObject(this, &ThisClass::OnVariantSpawnFinished);
		DiaSkillManagerComp->SpawnSkillActorUseVariants(CachedVariantContext, Ability, SpawnFinished);

		DrawDebugLine(World, CharacterLocation, CharacterLocation + LaunchDirection * 500.0f, FColor::Red, false, 1.5f, 0, 2.0f); // 캐릭터→LaunchDirection
		return;
	}

	OnFinished.ExecuteIfBound();
}

void UDiaProjectileSpawnStepExecutor::OnVariantSpawnFinished()
{
	if (CachedExecutionContext)
	{
		for (ADiaSkillActor* SpawnedActor : CachedVariantContext.SkillActors)
		{
			if (SpawnedActor)
			{
				CachedExecutionContext->SpawnedActors.Add(SpawnedActor);
				CachedExecutionContext->LastStepLocation = SpawnedActor->GetActorLocation();
			}
		}
	}

	CachedExecutionContext = nullptr;
	CachedVariantContext = FDiaSkillVariantContext();
	FDiaSkillStepFinishedDelegate Finished = CachedOnFinished;
	CachedOnFinished.Unbind();
	Finished.ExecuteIfBound();
}

FVector UDiaProjectileSpawnStepExecutor::CalcSpawnLocation(const FGameplayAbilityActorInfo& ActorInfo, const FGASProjectileData* ProjectileData)
{
	if (!ActorInfo.AvatarActor.IsValid())
	{
		return FVector::ForwardVector;
	}
	if (!ProjectileData)
	{
		return FVector::ForwardVector;
	}

	ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(ActorInfo.AvatarActor.Get());
	if (!Character)
	{
		return FVector::ForwardVector;
	}

	if (ProjectileData->bUseOwnerRotation)
	{
		// Use character's forward direction
		return Character->GetActorForwardVector();
	}
	else
	{
		FVector MouseWorldLocation = UDiaGASHelper::GetMouseWorldLocation(ActorInfo);
		FVector CharacterLocation = Character->GetActorLocation();

		// 마우스 위치까지의 2D 벡터
		FVector ToMouse = MouseWorldLocation - CharacterLocation;
		ToMouse.Z = 0.0f;

		DrawDebugLine(GetWorld(), CharacterLocation, MouseWorldLocation, FColor::Blue, false, 1.5f, 0, 2.0f);

		const float Distance = ToMouse.Size();
		if (Distance < KINDA_SMALL_NUMBER)
		{
			// 마우스 위치가 캐릭터와 거의 같으면 전방으로 발사
			return Character->GetActorForwardVector();
		}

		FVector Direction = ToMouse.GetSafeNormal();
		return Direction;
	}
}
