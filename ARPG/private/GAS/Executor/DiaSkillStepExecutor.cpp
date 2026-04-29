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

#include "Skill/DiaSkillActor.h"

bool UDiaChargeStepExecutor::CanExecute(const FInstancedStruct& StepData) const
{
    if(StepData.GetScriptStruct() == FGASChargeStepData::StaticStruct())
    {
        return true;
	}
    return false;
}

void UDiaChargeStepExecutor::Execute(const FInstancedStruct& StepData, UDiaGameplayAbility* Ability, FDiaSkillExecutionContext& Context, TFunction<void()> OnFinished)
{
	const FGASChargeStepData* CharStep = StepData.GetPtr<FGASChargeStepData>();
	if (!CharStep || !Ability)
	{
		OnFinished();
		return;
	}

	const FGASChargeData& ChargeData = CharStep->ChargeData;
	if (!ChargeData.PathOffsetCurve)
	{
		OnFinished();
		return;
	}

	CachedAbility = Ability;
	const FGameplayAbilityActorInfo& CurrentActorInfo = CachedAbility->GetActorInfo();

	CacheOnFinished = MoveTemp(OnFinished);


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
		CacheOnFinished();
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
	FVector DistLoc = StartLoc + ForwardVector * 600.f;

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
				UE_LOG(LogTemp, Log, TEXT("[ChargeAbility] Hit: %s at %s"),
					*HitActor->GetName(), *Hit.ImpactPoint.ToString());

				//ApplyHitToActorsInPath(HitActor);
			}
		}
	}

	if(CacheOnFinished)
	{
		CacheOnFinished();
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

void UDiaGroundSpawnStepExecutor::Execute(const FInstancedStruct& StepData, UDiaGameplayAbility* Ability, FDiaSkillExecutionContext& Context, TFunction<void()> OnFinished)
{
	const FGASGroundSpawnStepData* GroundStep = StepData.GetPtr<FGASGroundSpawnStepData>();
	if (!GroundStep || !Ability)
	{
		OnFinished();
		return;
	}

	const FGASGroundData& GroundData = GroundStep->GroundData;
	if (!GroundData.SkillActorClass)
	{
		OnFinished();
		return;
	}

	ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(Ability->GetAvatarActorFromActorInfo());
	if (!Character)
	{
		OnFinished();
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
		DiaSkillManagerComp->SpawnSkillActorUseVariants(VariantContext, Ability);

		for (ADiaSkillActor* SpawnedActor : VariantContext.SkillActors)
		{
			if (SpawnedActor)
			{
				Context.SpawnedActors.Add(SpawnedActor);
				Context.LastStepLocation = SpawnedActor->GetActorLocation();
			}
		}
	}

	OnFinished();
}

bool UDiaMeleeSpawnStepExecutor::CanExecute(const FInstancedStruct& StepData) const
{
	if (StepData.GetScriptStruct() == FGASMeleeSpawnStepData::StaticStruct())
	{
		return true;
	}

	return false;
}

void UDiaMeleeSpawnStepExecutor::Execute(const FInstancedStruct& StepData, UDiaGameplayAbility* Ability, FDiaSkillExecutionContext& Context, TFunction<void()> OnFinished)
{
}

bool UDiaProjectileSpawnStepExecutor::CanExecute(const FInstancedStruct& StepData) const
{
	if (StepData.GetScriptStruct() == FGASProjectileSpawnStepData::StaticStruct())
	{
		return true;
	}

	return false;
}

void UDiaProjectileSpawnStepExecutor::Execute(const FInstancedStruct& StepData, UDiaGameplayAbility* Ability, FDiaSkillExecutionContext& Context, TFunction<void()> OnFinished)
{
	if (!Ability)
	{
		OnFinished();
		return;
	}

	const FGASProjectileSpawnStepData* ProjectileStep = StepData.GetPtr<FGASProjectileSpawnStepData>();
	if (!ProjectileStep)
	{
		OnFinished();
		return;
	}

	const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
	const FGASProjectileData* ProjectileData = &ProjectileStep->ProjectileData;

	if (!ActorInfo.AvatarActor.IsValid() || !ProjectileData->SkillActorClass)
	{
		OnFinished();
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get());
	if (!Character)
	{
		OnFinished();
		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World)
	{
		OnFinished();
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

		UDiaSkillManagerComponent* DiaSkillManagerComp = Character->FindComponentByClass<UDiaSkillManagerComponent>();
		if (!DiaSkillManagerComp)
		{
			OnFinished();
			return;
		}

		DiaSkillManagerComp->SpawnSkillActorUseVariants(VariantContext, Ability);

		for (ADiaSkillActor* SpawnedActor : VariantContext.SkillActors)
		{
			if (SpawnedActor)
			{
				Context.SpawnedActors.Add(SpawnedActor);
				Context.LastStepLocation = SpawnedActor->GetActorLocation();
			}
		}

		DrawDebugLine(World, CharacterLocation, CharacterLocation + LaunchDirection * 500.0f, FColor::Red, false, 1.5f, 0, 2.0f); // 캐릭터→LaunchDirection
	}

	OnFinished();
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
