#include "GAS/Abilities/DiaProjectileAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Skill/DiaProjectile.h"
#include "Abilities/Tasks/AbilityTask_SpawnActor.h"
#include "GAS/DiaGASHelper.h"

UDiaProjectileAbility::UDiaProjectileAbility()
{
	// Set default values
	ProjectileOffset = FVector(50.0f, 0.0f, 0.0f);
	bUseOwnerRotation = false;
	bFireMultipleProjectiles = false;
	ProjectileCount = 3;
	SpreadAngle = 15.0f;

	// Set ability tags - 신규 API 사용
	//{
	//	FGameplayTagContainer Tags;
	//	Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Projectile")));
	//	SetAssetTags(Tags);
	//}
	//
	//FGameplayTagContainer BlockTags;
	//BlockTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));
	//BlockAbilitiesWithTag = BlockTags;
}

void UDiaProjectileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Call parent implementation
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Spawn projectile(s) immediately after ability activation
	SpawnProjectile();
}

void UDiaProjectileAbility::OnSpawned(AActor* SpawnedProjectile)
{
	ADiaProjectile* Projectile = Cast<ADiaProjectile>(SpawnedProjectile);
	UE_LOG(LogTemp, Log, TEXT("DiaProjectileAbility::OnSpawned - Projectile spawned"));
	if (Projectile)
	{
		UE_LOG(LogTemp, Log, TEXT("DiaProjectileAbility::OnSpawned - Projectile spawned successfully"));
		const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
		ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get());
		if (Character)
		{
			// Calculate launch direction
			FVector LaunchDirection = CalculateLaunchDirection(Character);
			if (LaunchDirection.IsNearlyZero())
			{
				LaunchDirection = Character->GetActorForwardVector();
			}
			LaunchDirection.Z = 0.0f;
			LaunchDirection.Normalize();


			const FGameplayAbilityActorInfo& Info = GetActorInfo();
			UAbilitySystemComponent* SourceASC = Info.AbilitySystemComponent.Get();

			//상대방에게 전달할 이펙트 생성
			TArray<FGameplayEffectSpecHandle> TargetEffectSpecs; 
			MakeEffectSpecContextToTarget(TargetEffectSpecs);
			Projectile->InitTargetEffectHandle(TargetEffectSpecs);
			Projectile->Initialize(SkillData.BaseDamage, Character, SourceASC, DamageEffectClass);
			Projectile->Launch(LaunchDirection);

		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDiaProjectileAbility::OnDidNotSpawn(AActor* SpawnedProjectile)
{
	UE_LOG(LogTemp, Warning, TEXT("DiaProjectileAbility::OnDidNotSpawn - Projectile failed to spawn"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDiaProjectileAbility::SpawnProjectile()
{
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!ProjectileClass || !ActorInfo.AvatarActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaProjectileAbility::SpawnProjectile - Cannot spawn projectile: missing requirements"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get());
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaProjectileAbility::SpawnProjectile - Cannot spawn projectile: not a character"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	// Calculate launch direction (수평 방향으로 정규화 + 최소 거리 보정)
	FVector LaunchDirection = CalculateLaunchDirection(Character);
	if (LaunchDirection.IsNearlyZero())
	{
		// 마우스 위치가 애매하거나 계산에 실패하면 캐릭터 전방으로 발사
		LaunchDirection = Character->GetActorForwardVector();
	}
	LaunchDirection.Z = 0.0f;
	LaunchDirection.Normalize();

	// Calculate spawn location: 발사 방향으로 일정 거리 앞
	FVector CharacterLocation = Character->GetActorLocation();
	const float SpawnDistance = FMath::Max(ProjectileOffset.Size(), MinimumRange);
	FVector SpawnLocation = CharacterLocation + LaunchDirection * SpawnDistance;

#if UE_EDITOR
	DrawDebugLine(World, CharacterLocation, SpawnLocation, FColor::Green, false, 1.5f, 0, 2.0f);
#endif

	if(IsValid(ProjectileClass))
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle;

		FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
		LocationData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocationData->SourceLocation.LiteralTransform = FTransform(LaunchDirection.Rotation(), SpawnLocation);
		LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocationData->TargetLocation.LiteralTransform = FTransform(LaunchDirection.Rotation(), SpawnLocation + LaunchDirection * 100.0f); // 약간 앞쪽
		TargetDataHandle.Add(LocationData);
		//SpawnTask라는걸 알내서 그걸 활용
		SpawnActorTask = UAbilityTask_SpawnActor::SpawnActor(this, TargetDataHandle, ProjectileClass);

		if (SpawnActorTask)
		{
			SpawnActorTask->Success.AddDynamic(this, &UDiaProjectileAbility::OnSpawned);
			SpawnActorTask->DidNotSpawn.AddDynamic(this, &UDiaProjectileAbility::OnDidNotSpawn);

			// C++에서는 BeginSpawningActor를 호출해야 실제로 스폰이 시작됩니다.
			AActor* SpawnedActor = nullptr;
			if (!SpawnActorTask->BeginSpawningActor(this, TargetDataHandle, ProjectileClass, SpawnedActor))
			{
				UE_LOG(LogTemp, Warning, TEXT("DiaProjectileAbility::SpawnProjectile - BeginSpawningActor failed"));
			}
			if (SpawnedActor)
			{
				SpawnActorTask->FinishSpawningActor(this, TargetDataHandle, SpawnedActor);
			}

			// ReadyForActivation은 Begin/Finish 이후에 호출해야 합니다
			SpawnActorTask->ReadyForActivation();
		}

		DrawDebugLine(World, CharacterLocation, CharacterLocation + LaunchDirection * 500.0f, FColor::Red, false, 1.5f, 0, 2.0f); // 캐릭터→LaunchDirection
	}
}

void UDiaProjectileAbility::InitializeWithSkillData(const FGASSkillData& InSkillData)
{
    Super::InitializeWithSkillData(InSkillData);
    if (InSkillData.SkillObjectClass)
    {
        ProjectileClass = InSkillData.SkillObjectClass;
    }
    // Multi-shot 같은 값은 데이터 구조에 따라 매핑 (예시로 Count/Spread를 Range/Radius로 매핑)
    if (InSkillData.SkillType == EGASSkillType::RangedAttack || InSkillData.SkillType == EGASSkillType::Magic)
    {
        // 선택적 매핑 규칙: Range -> ProjectileCount(정수 변환), Radius -> SpreadAngle
        ProjectileCount = FMath::Clamp(static_cast<int32>(InSkillData.Range / 200.0f), 1, 7);
        bFireMultipleProjectiles = ProjectileCount > 1;
        SpreadAngle = FMath::Clamp(InSkillData.Radius * 0.2f, 5.0f, 45.0f);
    }
}

FVector UDiaProjectileAbility::CalculateLaunchDirection(ACharacter* Character) const
{
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!ActorInfo.AvatarActor.IsValid())
	{
		return FVector::ForwardVector;
	}

	if (!Character)
	{
		return FVector::ForwardVector;
	}

	if (bUseOwnerRotation)
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

		DrawDebugLine(GetWorld(), CharacterLocation, MouseWorldLocation, FColor::Blue, false, 1.5f, 0, 2.0f);          // 캐릭터→마우스

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
//
//FVector UDiaProjectileAbility::GetMouseWorldLocation() const
//{
//	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
//	if (!ActorInfo.PlayerController.IsValid())
//	{
//		return FVector::ZeroVector;
//	}
//
//	APlayerController* PC = ActorInfo.PlayerController.Get();
//	
//	FVector MouseWorldLocation, MouseWorldDirection;
//	bool bDeprojectSuccess = PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection);
//	
//	if (!bDeprojectSuccess)
//	{
//		return FVector::ZeroVector;
//	}
//
//	// Trace from mouse position to ground
//	FHitResult HitResult;
//	FVector TraceStart = MouseWorldLocation;
//	FVector TraceEnd = MouseWorldLocation + MouseWorldDirection * 10000.0f; // Trace far distance
//
//	FCollisionQueryParams QueryParams;
//	QueryParams.AddIgnoredActor(ActorInfo.AvatarActor.Get());
//
//	bool bHit = GetWorld()->LineTraceSingleByChannel(
//		HitResult,
//		TraceStart,
//		TraceEnd,
//		ECC_WorldStatic,
//		QueryParams
//	);
//
//	if (bHit)
//	{
//		return HitResult.Location;
//	}
//
//	// If no hit, project to character's Z level
//	ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get());
//	if (Character)
//	{
//		float CharacterZ = Character->GetActorLocation().Z;
//		float T = (CharacterZ - MouseWorldLocation.Z) / MouseWorldDirection.Z;
//		return MouseWorldLocation + MouseWorldDirection * T;
//	}
//
//	return FVector::ZeroVector;
//}