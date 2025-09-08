#include "GAS/Abilities/DiaMeleeAbility.h"
#include "GAS/DiaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

UDiaMeleeAbility::UDiaMeleeAbility()
{
	// Set default values
	AttackRange = 200.0f;
	AttackAngle = 60.0f;
	AttackOffset = FVector(100.0f, 0.0f, 0.0f);
	bShowDebugShape = false;

	// Set ability tags
	FGameplayTagContainer Tags;
	Tags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Melee")));
	SetAssetTags(Tags);
	
	FGameplayTagContainer BlockTags;
	BlockTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability")));
	BlockAbilitiesWithTag = BlockTags;
}

void UDiaMeleeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// Clear hit actors for this activation
	HitActors.Empty();

	// Call parent implementation
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UDiaMeleeAbility::PerformHitDetection()
{
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!ActorInfo.AvatarActor.IsValid())
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(ActorInfo.AvatarActor.Get());
	if (!Character)
	{
		return;
	}

	UWorld* World = Character->GetWorld();
	if (!World)
	{
		return;
	}

	// Get character location and rotation
	FVector CharacterLocation = Character->GetActorLocation();
	FRotator CharacterRotation = Character->GetActorRotation();

	// Calculate attack center position
	FVector AttackCenter = CharacterLocation + CharacterRotation.RotateVector(AttackOffset);

	// Perform sphere sweep
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

	// Debug drawing
	if (bShowDebugShape)
	{
		DrawDebugSphere(World, AttackCenter, AttackRange, 12, FColor::Red, false, 2.0f);
		
		// Draw attack cone
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

			// Check if target is within attack angle
			FVector ToTarget = (HitActor->GetActorLocation() - AttackCenter).GetSafeNormal();
			float DotProduct = FVector::DotProduct(ForwardVector, ToTarget);
			float AngleToTarget = FMath::Acos(DotProduct);

			if (AngleToTarget <= HalfAngleRad)
			{
				// Add to hit actors to prevent multiple hits
				HitActors.Add(HitActor);

				// Apply damage
				ApplyDamageToTarget(HitActor);

				// Play hit effects
				if (HitEffect)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						World,
						HitEffect,
						HitResult.Location
					);
				}

				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						World,
						HitSound,
						HitResult.Location
					);
				}
			}
		}
	}
}

void UDiaMeleeAbility::OnMeleeHitFrame()
{
	// This function is called from animation notify
	PerformHitDetection();
}

void UDiaMeleeAbility::ApplyDamageToTarget(AActor* Target)
{
	const FGameplayAbilityActorInfo& ActorInfo = GetActorInfo();
	if (!Target || !ActorInfo.AbilitySystemComponent.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = Target->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC)
	{
		// If target doesn't have ASC, apply damage directly
		float DamageAmount = SkillData.BaseDamage;
		
		// Apply damage using UE's damage system
		UGameplayStatics::ApplyPointDamage(
			Target,
			DamageAmount,
			ActorInfo.AvatarActor->GetActorLocation(),
			FHitResult(),
			ActorInfo.PlayerController.Get(),
			ActorInfo.AvatarActor.Get(),
			UDamageType::StaticClass()
		);
		return;
	}

	// Apply damage through GAS
	// For now, we'll apply damage directly to the attribute
	// In a full implementation, you'd create and apply a GameplayEffect
	UDiaAttributeSet* TargetAttributeSet = const_cast<UDiaAttributeSet*>(TargetASC->GetSet<UDiaAttributeSet>());
	if (TargetAttributeSet)
	{
		float CurrentHealth = TargetAttributeSet->GetHealth();
		float DamageAmount = SkillData.BaseDamage;
		
		// Get attack power from our character
		if (const UDiaAttributeSet* MyAttributeSet = GetAbilitySystemComponentFromActorInfo()->GetSet<UDiaAttributeSet>())
		{
			DamageAmount += MyAttributeSet->GetAttackPower();
		}

		// Apply defense
		DamageAmount = FMath::Max(1.0f, DamageAmount - TargetAttributeSet->GetDefense());
		
		float NewHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
		TargetAttributeSet->SetHealth(NewHealth);

		UE_LOG(LogTemp, Warning, TEXT("Melee damage applied: %f, Target health: %f"), DamageAmount, NewHealth);
	}
}