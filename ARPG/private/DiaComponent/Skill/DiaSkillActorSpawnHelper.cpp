// Fill out your copyright notice in the Description page of Project Settings.

#include "DiaComponent/Skill/DiaSkillActorSpawnHelper.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GAS/DiaGameplayAbility.h"
#include "Skill/DiaSkillActor.h"
#include "Logging/ARPGLogChannels.h"

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
	ARPG_SKILL_VLOG(TEXT("SpawnSkillActor request. World=%s, Class=%s, Owner=%s, Instigator=%s, SkillData=%s, SourceASC=%s, DamageEffect=%s, OwningAbility=%s, Location=%s, Rotation=%s, TargetEffects=%d, IgnoredActors=%d, bSpawnedByFork=%s, bLaunch=%s, bApplyRemovalTimer=%s"),
		Request.World ? TEXT("Valid") : TEXT("None"),
		*GetNameSafe(Request.SkillActorClass),
		*GetNameSafe(Request.OwnerActor),
		*GetNameSafe(Request.Instigator),
		Request.SkillData ? TEXT("Valid") : TEXT("None"),
		*GetNameSafe(Request.SourceASC),
		*GetNameSafe(Request.DamageEffectClass),
		*GetNameSafe(Request.OwningAbility),
		*Request.SpawnTransform.GetLocation().ToString(),
		*Request.SpawnTransform.GetRotation().Rotator().ToString(),
		Request.TargetEffectSpecs.Num(),
		Request.IgnoredActors.Num(),
		Request.bSpawnedByFork ? TEXT("true") : TEXT("false"),
		Request.bLaunch ? TEXT("true") : TEXT("false"),
		Request.bApplyAbilityRemovalTimer ? TEXT("true") : TEXT("false"));

	if (!Request.World || !Request.SkillActorClass || !Request.SkillData)
	{
		ARPG_SKILL_LOG(Warning, TEXT("SpawnSkillActor failed: invalid request. World=%s, Class=%s, SkillData=%s, Owner=%s, SourceASC=%s, Location=%s"),
			Request.World ? TEXT("Valid") : TEXT("None"),
			*GetNameSafe(Request.SkillActorClass),
			Request.SkillData ? TEXT("Valid") : TEXT("None"),
			*GetNameSafe(Request.OwnerActor),
			*GetNameSafe(Request.SourceASC),
			*Request.SpawnTransform.GetLocation().ToString());
		return nullptr;
	}

	if (!Request.OwnerActor)
	{
		ARPG_SKILL_LOG(Warning, TEXT("SpawnSkillActor warning: OwnerActor is null. Class=%s, SkillID=%d"),
			*GetNameSafe(Request.SkillActorClass),
			Request.SkillData->SkillID);
	}

	if (!Request.SourceASC)
	{
		ARPG_SKILL_LOG(Warning, TEXT("SpawnSkillActor warning: SourceASC is null. Class=%s, SkillID=%d, Owner=%s"),
			*GetNameSafe(Request.SkillActorClass),
			Request.SkillData->SkillID,
			*GetNameSafe(Request.OwnerActor));
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
		ARPG_SKILL_LOG(Warning, TEXT("SpawnSkillActor failed: SpawnActorDeferred returned null. Class=%s, SkillID=%d, Owner=%s, Instigator=%s, Location=%s, Rotation=%s"),
			*GetNameSafe(Request.SkillActorClass),
			Request.SkillData->SkillID,
			*GetNameSafe(Request.OwnerActor),
			*GetNameSafe(Request.Instigator),
			*Request.SpawnTransform.GetLocation().ToString(),
			*Request.SpawnTransform.GetRotation().Rotator().ToString());
		return nullptr;
	}

	ARPG_SKILL_VLOG(TEXT("SpawnSkillActor deferred spawn success. Actor=%s, Class=%s, SkillID=%d"),
		*GetNameSafe(SpawnedActor),
		*GetNameSafe(Request.SkillActorClass),
		Request.SkillData->SkillID);

	ARPG_SKILL_VLOG(TEXT("SpawnSkillActor step: InitTargetEffectHandle. Actor=%s, TargetEffects=%d"),
		*GetNameSafe(SpawnedActor),
		Request.TargetEffectSpecs.Num());
	SpawnedActor->InitTargetEffectHandle(Request.TargetEffectSpecs);

	ARPG_SKILL_VLOG(TEXT("SpawnSkillActor step: Initialize. Actor=%s, SkillID=%d, Owner=%s, SourceASC=%s, DamageEffect=%s"),
		*GetNameSafe(SpawnedActor),
		Request.SkillData->SkillID,
		*GetNameSafe(Request.OwnerActor),
		*GetNameSafe(Request.SourceASC),
		*GetNameSafe(Request.DamageEffectClass));
	SpawnedActor->Initialize(*Request.SkillData, Request.OwnerActor, Request.SourceASC, Request.DamageEffectClass);

	ARPG_SKILL_VLOG(TEXT("SpawnSkillActor step: AddIgnoredHitActors. Actor=%s, IgnoredActors=%d"),
		*GetNameSafe(SpawnedActor),
		Request.IgnoredActors.Num());
	SpawnedActor->AddIgnoredHitActors(Request.IgnoredActors);

	ARPG_SKILL_VLOG(TEXT("SpawnSkillActor step: SetOwner/Ability/Runtime/Fork. Actor=%s, Owner=%s, OwningAbility=%s, bSpawnedByFork=%s"),
		*GetNameSafe(SpawnedActor),
		*GetNameSafe(Request.OwnerActor),
		*GetNameSafe(Request.OwningAbility),
		Request.bSpawnedByFork ? TEXT("true") : TEXT("false"));
	SpawnedActor->SetOwner(Request.OwnerActor);
	SpawnedActor->SetOwningAbility(Request.OwningAbility);
	SpawnedActor->ApplySpawnRuntimeParams(Request.RuntimeParams);
	SpawnedActor->SetSpawnedByFork(Request.bSpawnedByFork);

	ARPG_SKILL_VLOG(TEXT("SpawnSkillActor step: FinishSpawning. Actor=%s, Location=%s, Rotation=%s"),
		*GetNameSafe(SpawnedActor),
		*Request.SpawnTransform.GetLocation().ToString(),
		*Request.SpawnTransform.GetRotation().Rotator().ToString());
	SpawnedActor->FinishSpawning(Request.SpawnTransform);

	if (Request.bLaunch)
	{
		ARPG_SKILL_VLOG(TEXT("SpawnSkillActor step: Launch. Actor=%s, Direction=%s"),
			*GetNameSafe(SpawnedActor),
			*Request.SpawnTransform.GetRotation().GetForwardVector().ToString());
		SpawnedActor->Launch(Request.SpawnTransform.GetRotation().GetForwardVector());
	}

	if (Request.bApplyAbilityRemovalTimer && Request.OwningAbility)
	{
		ARPG_SKILL_VLOG(TEXT("SpawnSkillActor step: ApplySkillObjectRemovalTimer. Actor=%s, OwningAbility=%s"),
			*GetNameSafe(SpawnedActor),
			*GetNameSafe(Request.OwningAbility));
		Request.OwningAbility->ApplySkillObjectRemovalTimer(SpawnedActor);
	}

	ARPG_SKILL_VLOG(TEXT("SpawnSkillActor success. Actor=%s, SkillID=%d, Class=%s"),
		*GetNameSafe(SpawnedActor),
		Request.SkillData->SkillID,
		*GetNameSafe(Request.SkillActorClass));

	return SpawnedActor;
}
