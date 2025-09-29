#include "GAS/Abilities/DiaMeleeAbility.h"
#include "GAS/DiaAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "GameFramework/Character.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Character/DiaCharacter.h"
#include "DiaBaseCharacter.h"

#include "Components/AudioComponent.h"

UDiaMeleeAbility::UDiaMeleeAbility()
{
	// Set default values
	AttackRange = 200.0f;
	AttackAngle = 60.0f;
	AttackOffset = FVector(100.0f, 0.0f, 0.0f);

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
	HitActors.Empty();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	PerformHitDetection();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
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
				HitActors.Add(HitActor);

                ApplyDamageToTarget(HitActor);

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
        // ASC가 없으면 기존 방식으로 최소한의 대미지 처리
        const float DamageAmount = SkillData.BaseDamage;
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


	//피격 판정 후 로그 작성
	UE_LOG(LogTemp, Log, TEXT("Melee Hit: %s"), *Target->GetName());

    // GAS 경로: Execution 기반 대미지 적용
    const UDiaAttributeSet* MyAttr = GetAbilitySystemComponentFromActorInfo()->GetSet<UDiaAttributeSet>();
    const float AttackPower = MyAttr ? MyAttr->GetAttackPower() : 0.f;
    const float BaseDamage = SkillData.BaseDamage + AttackPower;
    ApplyDamageToASC(TargetASC, BaseDamage , 1.0f);
}