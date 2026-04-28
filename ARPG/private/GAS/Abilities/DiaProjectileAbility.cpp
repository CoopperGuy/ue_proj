#include "GAS/Abilities/DiaProjectileAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Skill/DiaProjectile.h"
#include "Abilities/Tasks/AbilityTask_SpawnActor.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
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

	if (SkillData.CastTime == 0.f)
	{
		SpawnProjectile();
	}
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
		UE_LOG(LogTemp, Warning, TEXT("DiaProjectileAbility::SpawnProjectile - Cannot spawn projectile. SkillID: %d, ProjectileClass: %s, AvatarValid: %s"),
			SkillData.SkillID,
			*GetNameSafe(ProjectileClass),
			ActorInfo.AvatarActor.IsValid() ? TEXT("true") : TEXT("false"));
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

	UE_LOG(LogTemp, Log, TEXT("DiaProjectileAbility::SpawnProjectile - SkillID: %d, Class: %s, Location: %s, Direction: %s"),
		SkillData.SkillID,
		*GetNameSafe(ProjectileClass),
		*SpawnLocation.ToString(),
		*LaunchDirection.ToString());

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

		FDiaSkillVariantContext VariantContext;
		VariantContext.SkillActorClass = ProjectileClass;
		VariantContext.TargetData = TargetDataHandle;

		UDiaSkillManagerComponent* DiaSkillManagerComp = Character->FindComponentByClass<UDiaSkillManagerComponent>();
		DiaSkillManagerComp->SpawnSkillActorUseVariants(VariantContext, this);
		
		DrawDebugLine(World, CharacterLocation, CharacterLocation + LaunchDirection * 500.0f, FColor::Red, false, 1.5f, 0, 2.0f); // 캐릭터→LaunchDirection
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDiaProjectileAbility::ProcessSkillDelayEvents()
{
	Super::ProcessSkillDelayEvents();

	UE_LOG(LogTemp, Log, TEXT("DiaProjectileAbility::ProcessSkillDelayEvents - SkillID: %d"), SkillData.SkillID);

	// Spawn projectile(s) immediately after ability activation
	SpawnProjectile();
}

bool UDiaProjectileAbility::ShouldEndAbilityOnMontageCompleted() const
{
	return SkillData.CastTime <= 0.f;
}

void UDiaProjectileAbility::InitializeWithSkillData(const FGASSkillData& InSkillData)
{
    Super::InitializeWithSkillData(InSkillData);

    const FGASProjectileData* P = InSkillData.GetExtraPtr<FGASProjectileData>();
    if (!P)
    {
        return;
    }

    if (P->SkillActorClass)
    {
        ProjectileClass = P->SkillActorClass;
    }
    ProjectileOffset = P->ProjectileOffset;
    MinimumRange = P->MinimumRange;
    bUseOwnerRotation = P->bUseOwnerRotation;
    bFireMultipleProjectiles = P->bFireMultipleProjectiles;
    ProjectileCount = P->ProjectileCount;
    SpreadAngle = P->SpreadAngle;
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

