#include "GAS/Abilities/DiaProjectileAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Skill/DiaProjectile.h"

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
	FVector LaunchDirection = CalculateLaunchDirection();
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
	UE_LOG(LogTemp, Verbose, TEXT("DiaProjectileAbility: Calc Spawn | Char=%s Spawn=%s Dir=%s Dist=%.1f"),
		*CharacterLocation.ToString(), *SpawnLocation.ToString(), *LaunchDirection.ToString(), SpawnDistance);
#endif

	if (bFireMultipleProjectiles && ProjectileCount > 1)
	{
		// Fire multiple projectiles with spread
		float HalfSpread = SpreadAngle * 0.5f;
		float AngleStep = SpreadAngle / (ProjectileCount - 1);

		for (int32 i = 0; i < ProjectileCount; ++i)
		{
			float CurrentAngle = -HalfSpread + (AngleStep * i);
			FVector CurrentDirection = LaunchDirection.RotateAngleAxis(CurrentAngle, FVector::UpVector);
			FRotator SpawnRotation = CurrentDirection.Rotation();

			// Spawn projectile
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Character;
			SpawnParams.Instigator = Character;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ADiaProjectile* Projectile = World->SpawnActor<ADiaProjectile>(
				ProjectileClass,
				SpawnLocation,
				SpawnRotation,
				SpawnParams
			);

            if (Projectile)
            {
                // GAS 초기화: 소스 ASC, 대미지 GE, 기본 대미지 전달
                const FGameplayAbilityActorInfo& Info = GetActorInfo();
                UAbilitySystemComponent* SourceASC = Info.AbilitySystemComponent.Get();
                Projectile->Initialize(SkillData.BaseDamage, Character, SourceASC, DamageEffectClass);
                Projectile->Launch(CurrentDirection);
				UE_LOG(LogTemp, Log, TEXT("DiaProjectileAbility: Spawned projectile %d | Spawn=%s Dir=%s"), i, *SpawnLocation.ToString(), *CurrentDirection.ToString());
            }
		}
	}
	else
	{
		// Fire single projectile
		FRotator SpawnRotation = LaunchDirection.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ADiaProjectile* Projectile = World->SpawnActor<ADiaProjectile>(
			ProjectileClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

        if (Projectile)
        {
            const FGameplayAbilityActorInfo& Info = GetActorInfo();
            UAbilitySystemComponent* SourceASC = Info.AbilitySystemComponent.Get();
            Projectile->Initialize(SkillData.BaseDamage, Character, SourceASC, DamageEffectClass);
            Projectile->Launch(LaunchDirection);
            UE_LOG(LogTemp, Log, TEXT("DiaProjectileAbility: Spawned single projectile | Spawn=%s Dir=%s"), *SpawnLocation.ToString(), *LaunchDirection.ToString());
        }
	}

	// End ability after spawning projectiles
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDiaProjectileAbility::InitializeWithSkillData(const FGASSkillData& InSkillData)
{
    Super::InitializeWithSkillData(InSkillData);
    if (InSkillData.ProjectileClass)
    {
        ProjectileClass = InSkillData.ProjectileClass;
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

FVector UDiaProjectileAbility::CalculateLaunchDirection() const
{
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!ActorInfo.AvatarActor.IsValid())
	{
		return FVector::ForwardVector;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get());
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
		// Use direction towards mouse cursor (너무 가까운 경우/불안정한 방향은 보정)
		FVector MouseWorldLocation = GetMouseWorldLocation();
		FVector CharacterLocation = Character->GetActorLocation();

		// 마우스 위치까지의 2D 벡터
		FVector ToMouse = MouseWorldLocation - CharacterLocation;
		ToMouse.Z = 0.0f;

		const float Distance = ToMouse.Size();
		if (Distance < KINDA_SMALL_NUMBER)
		{
			// 마우스 위치가 캐릭터와 거의 같으면 전방으로 발사
			return Character->GetActorForwardVector();
		}

		// 너무 가까운 지점을 클릭한 경우 최소 사거리만큼 앞으로 보정
		if (Distance < MinimumRange)
		{
			ToMouse = ToMouse.GetSafeNormal() * MinimumRange;
		}

		FVector Direction = ToMouse.GetSafeNormal();
		return Direction;
	}
}

FVector UDiaProjectileAbility::GetMouseWorldLocation() const
{
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!ActorInfo.PlayerController.IsValid())
	{
		return FVector::ZeroVector;
	}

	APlayerController* PC = ActorInfo.PlayerController.Get();
	
	FVector MouseWorldLocation, MouseWorldDirection;
	bool bDeprojectSuccess = PC->DeprojectMousePositionToWorld(MouseWorldLocation, MouseWorldDirection);
	
	if (!bDeprojectSuccess)
	{
		return FVector::ZeroVector;
	}

	// Trace from mouse position to ground
	FHitResult HitResult;
	FVector TraceStart = MouseWorldLocation;
	FVector TraceEnd = MouseWorldLocation + MouseWorldDirection * 10000.0f; // Trace far distance

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(ActorInfo.AvatarActor.Get());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_WorldStatic,
		QueryParams
	);

	if (bHit)
	{
		return HitResult.Location;
	}

	// If no hit, project to character's Z level
	ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get());
	if (Character)
	{
		float CharacterZ = Character->GetActorLocation().Z;
		float T = (CharacterZ - MouseWorldLocation.Z) / MouseWorldDirection.Z;
		return MouseWorldLocation + MouseWorldDirection * T;
	}

	return FVector::ZeroVector;
}