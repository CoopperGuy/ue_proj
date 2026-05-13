// Fill out your copyright notice in the Description page of Project Settings.

#include "DiaComponent/Skill/DiaSkillActorSpawnHelper.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GAS/DiaGameplayAbility.h"
#include "Skill/DiaSkillActor.h"

FSpawnLocationData FDiaSkillActorSpawnHelper::CalculateSpawnLocationData(
	const FDiaSkillVariantContext& Context,
	const AActor* Character,
	const FSkillSpawnRuntime& Runtime)
{
	constexpr float AdditionalAngle = 10.f;

	FSpawnLocationData Result;

	const FVector CharacterForward = Character ? Character->GetActorForwardVector() : FVector::ForwardVector;
	Result.CharacterLocation = Character ? Character->GetActorLocation() : FVector::ZeroVector;
	Result.Direction = FVector::ForwardVector;
	Result.BaseSpawnTransform = FTransform(CharacterForward.Rotation(), Result.CharacterLocation + CharacterForward * 100.f);

	// 타겟 위치와 방향 가져오기
	if (const FGameplayAbilityTargetData* LocationData = Context.TargetData.Get(0))
	{
		if (LocationData->GetScriptStruct() == FGameplayAbilityTargetData_LocationInfo::StaticStruct())
		{
			const FGameplayAbilityTargetData_LocationInfo* LocationInfo = static_cast<const FGameplayAbilityTargetData_LocationInfo*>(LocationData);
			if (LocationInfo->SourceLocation.LocationType == EGameplayAbilityTargetingLocationType::LiteralTransform)
			{
				Result.BaseSpawnTransform = LocationInfo->SourceLocation.LiteralTransform;
				Result.bHasSpawnTransform = true;
			}

			if (LocationInfo->TargetLocation.LocationType == EGameplayAbilityTargetingLocationType::LiteralTransform)
			{
				const FVector TargetLocation = LocationInfo->TargetLocation.LiteralTransform.GetLocation();
				Result.Direction = (TargetLocation - Result.BaseSpawnTransform.GetLocation()).GetSafeNormal();
			}
		}
		else if (LocationData->HasHitResult())
		{
			const FVector TargetLocation = LocationData->GetHitResult()->Location;
			Result.Direction = (TargetLocation - Result.CharacterLocation).GetSafeNormal();
		}
		else if (LocationData->HasEndPoint())
		{
			const FVector TargetLocation = LocationData->GetEndPointTransform().GetLocation();
			Result.Direction = (TargetLocation - Result.CharacterLocation).GetSafeNormal();
		}
	}

	// 타겟 방향이 없으면 캐릭터의 앞 방향 사용
	if (Result.Direction.IsNearlyZero())
	{
		Result.Direction = CharacterForward;
	}

	Result.BaseSpawnTransform.SetRotation(Result.Direction.Rotation().Quaternion());

	const FRotator InitialRotator = FRotator(0.f, -AdditionalAngle * 0.5f * (Runtime.ExtraSpawnCount - 1), 0.f);
	Result.NextProjectileRotator = FRotator(0.f, AdditionalAngle, 0.f);
	Result.Direction = InitialRotator.RotateVector(Result.Direction).GetSafeNormal();

	return Result;
}

FTransform FDiaSkillActorSpawnHelper::MakeSpawnTransform(
	const FSpawnLocationData& SpawnLocationData,
	const FVector& Direction)
{
	FTransform SpawnTransform = SpawnLocationData.BaseSpawnTransform;
	SpawnTransform.SetRotation(Direction.Rotation().Quaternion());

	if (!SpawnLocationData.bHasSpawnTransform)
	{
		SpawnTransform.SetLocation(SpawnLocationData.CharacterLocation + Direction * 100.f);
	}

	return SpawnTransform;
}

void FDiaSkillActorSpawnHelper::ApplyVariantEffects(
	const TArray<UDiaSkillVariant*>& Variants,
	FDiaSkillVariantContext& Context,
	const UDiaGameplayAbility* Ability,
	FSkillSpawnRuntime& Runtime,
	TArray<FGameplayEffectSpecHandle>& TargetEffectSpecs)
{
	UAbilitySystemComponent* SourceASC = Ability->GetAbilitySystemComponentFromActorInfo();
	const FGameplayAbilityActorInfo* CurrentActorInfo = Ability->GetCurrentActorInfo();
	if (!IsValid(SourceASC))
		return;
	if (CurrentActorInfo == nullptr)
		return;

	for (const UDiaSkillVariant* Variant : Variants)
	{
		if (Variant == nullptr)
			continue;
		
		const FDiaSkillVariantSpec& Spec = Variant->GetVariantSpec();

		if (const FSkillVariantGameplayEffectData* VariantGameplayEffec = Spec.GetVariantExtraPtr<FSkillVariantGameplayEffectData>())
		{
			for (const auto& TagerEffect : VariantGameplayEffec->TargetEffects)
			{
				FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
				EffectContext.AddInstigator(CurrentActorInfo ? CurrentActorInfo->OwnerActor.Get() : nullptr,
					CurrentActorInfo ? Cast<APawn>(CurrentActorInfo->AvatarActor.Get()) : nullptr);

				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(TagerEffect.EffectClass, 1, EffectContext);

				if (SpecHandle.IsValid())
				{
					for (const auto& SetByCaller : TagerEffect.SetByCallerMagnitudes)
					{
						SpecHandle.Data.Get()->SetSetByCallerMagnitude(SetByCaller.Key, SetByCaller.Value);
					}
					
					TargetEffectSpecs.Add(SpecHandle);
				}
			}
		}
	}
}

ADiaSkillActor* FDiaSkillActorSpawnHelper::SpawnSkillActor(const FDiaSkillActorSpawnRequest& Request)
{
	if (!Request.World || !Request.SkillActorClass || !Request.SkillData)
	{
		UE_LOG(LogTemp, Warning, TEXT("FDiaSkillActorSpawnHelper::SpawnSkillActor - Invalid request. World: %s, Class: %s, SkillData: %s"),
			Request.World ? TEXT("Valid") : TEXT("None"),
			*GetNameSafe(Request.SkillActorClass),
			Request.SkillData ? TEXT("Valid") : TEXT("None"));
		return nullptr;
	}

	ADiaSkillActor* SpawnedActor = Request.World->SpawnActorDeferred<ADiaSkillActor>(
		Request.SkillActorClass,
		Request.SpawnTransform,
		Request.OwnerActor,
		Request.Instigator,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
	);

	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("FDiaSkillActorSpawnHelper::SpawnSkillActor - Spawn failed. Class: %s"),
			*GetNameSafe(Request.SkillActorClass));
		return nullptr;
	}

	SpawnedActor->InitTargetEffectHandle(Request.TargetEffectSpecs);
	SpawnedActor->Initialize(*Request.SkillData, Request.OwnerActor, Request.SourceASC, Request.DamageEffectClass);
	SpawnedActor->AddIgnoredHitActors(Request.IgnoredActors);
	SpawnedActor->SetOwner(Request.OwnerActor);
	SpawnedActor->SetOwningAbility(Request.OwningAbility);
	SpawnedActor->SetDamageMultiplier(Request.DamageMultiplier);
	SpawnedActor->SetPierceCount(FMath::Max(1, Request.PierceCount));
	SpawnedActor->SetSpawnedByFork(Request.bSpawnedByFork);

	SpawnedActor->FinishSpawning(Request.SpawnTransform);

	if (Request.bLaunch)
	{
		SpawnedActor->Launch(Request.SpawnTransform.GetRotation().GetForwardVector());
	}

	if (Request.bApplyAbilityRemovalTimer && Request.OwningAbility)
	{
		Request.OwningAbility->ApplySkillObjectRemovalTimer(SpawnedActor);
	}

	return SpawnedActor;
}
