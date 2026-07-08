// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Executor/DiaSkillVariantSpawnExecutor.h"
#include "DiaComponent/Skill/Effect/DiaVariantEffect_SpawnModifier.h"
#include "DiaComponent/Skill/DiaSkillActorSpawnHelper.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaComponent/Skill/SkillObject.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/World.h"
#include "GAS/DiaGameplayTags.h"

#include "Types/DiaGASSkillData.h"
#include "Skill/DiaSkillActor.h"
#include "GAS/DiaGameplayAbility.h"

#include "Types/DiaGASSkillData.h"

#include "DiaBaseCharacter.h"
#include "Logging/ARPGLogChannels.h"

void UDiaSkillVariantSpawnExecutor::InitializeExecutor()
{
	RegisteredEffects.Add(NewObject<UDiaVariantEffect_SpawnModifier>(this));
}

void UDiaSkillVariantSpawnExecutor::ExecuteEffect(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability)
{
	ExecuteEffect(Variants, Context, Ability, FDiaSkillSpawnFinishedDelegate());
}

void UDiaSkillVariantSpawnExecutor::ExecuteEffect(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability, FDiaSkillSpawnFinishedDelegate OnFinished)
{
	if (!IsValid(Ability))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - Ability is invalid"));
		OnFinished.ExecuteIfBound();
		return;
	}

	if(Context.SkillActorClass == nullptr)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - SkillActorClass is null"));
		OnFinished.ExecuteIfBound();
		return;
	}

	FSkillSpawnRuntime Runtime;
	Runtime.ExtraSpawnCount = FMath::Max(1, Context.BaseSpawnCount);
	ApplyEffects(Variants, Context, Runtime);	
	SpawnCast(Variants, Context, Ability, Runtime);
	if (AfterSpawn(Variants, Context, Ability, Runtime, OnFinished))
	{
		return;
	}

	OnFinished.ExecuteIfBound();
}

void UDiaSkillVariantSpawnExecutor::SpawnCast(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability, FSkillSpawnRuntime& Runtime) const
{
	if (!IsValid(Ability))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillVariantSpawnExecutor::SpawnCast - Ability is invalid"));
		return;
	}

	const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
	AActor* Character = (ActorInfo.AvatarActor.Get());
	APawn* Pawn = Cast<APawn>(Character);
	FSpawnLocationData SpawnLocationData = FDiaSkillActorSpawnHelper::CalculateSpawnLocationData(Context, Character, Runtime);
	FVector Direction = SpawnLocationData.Direction;

	UWorld* World = Ability ? Ability->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - World가 유효하지 않습니다."));
		return;
	}

	for (int32 i = 0; i < Runtime.ExtraSpawnCount; i++)
	{
		const FTransform SpawnTransform = FDiaSkillActorSpawnHelper::MakeSpawnTransform(SpawnLocationData, Direction);

		UE_LOG(LogARPG, Log, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - SkillID: %d, Class: %s, SpawnLocation: %s, Direction: %s"),
			Ability ? Ability->GetSkillData().SkillID : -1,
			*GetNameSafe(Context.SkillActorClass),
			*SpawnTransform.GetLocation().ToString(),
			*Direction.ToString());

		TArray<FGameplayEffectSpecHandle> TargetEffectSpecs;
		Ability->MakeEffectSpecContextToTarget(TargetEffectSpecs);
		FDiaSkillActorSpawnHelper::ApplyVariantEffects(Variants, Context, Ability, Runtime, TargetEffectSpecs);

		FDiaSkillActorSpawnRequest SpawnRequest;
		SpawnRequest.World = World;
		SpawnRequest.SkillActorClass = Context.SkillActorClass;
		SpawnRequest.SpawnTransform = SpawnTransform;
		SpawnRequest.OwnerActor = Character;
		SpawnRequest.Instigator = Pawn;
		SpawnRequest.SkillData = &Ability->GetSkillData();
		SpawnRequest.SourceASC = ActorInfo.AbilitySystemComponent.Get();
		SpawnRequest.DamageEffectClass = Ability->GetDamageEffectClass();
		SpawnRequest.OwningAbility = Ability;
		SpawnRequest.TargetEffectSpecs = MoveTemp(TargetEffectSpecs);
		SpawnRequest.RuntimeParams = Runtime.ActorParams;

		if (ADiaSkillActor* SpawnedActor = FDiaSkillActorSpawnHelper::SpawnSkillActor(SpawnRequest))
		{
			Context.SkillActors.Add(SpawnedActor);
		}
		// 다음 발사체를 위한 방향 업데이트 (다음 루프에서 사용)
		Direction = SpawnLocationData.NextProjectileRotator.RotateVector(Direction).GetSafeNormal();
	}
}

bool UDiaSkillVariantSpawnExecutor::AfterSpawn(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability, FSkillSpawnRuntime& Runtime, FDiaSkillSpawnFinishedDelegate OnFinished)
{
	UWorld* World = Ability ? Ability->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	int32 RemainingRepeatCount = 0;
	float RepeatDelay = 0.f;
	for(const UDiaSkillVariant* Variant : Variants)
	{
		if (Variant == nullptr)
			continue;
		const FDiaSkillVariantSpec& Spec = Variant->GetVariantSpec();
		if(Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_RepeatCast))
		{
			RemainingRepeatCount = 1;
			RepeatDelay = FMath::Max(0.01f, Spec.ModifierValue);
		}
	}

	if (RemainingRepeatCount <= 0)
	{
		return false;
	}

	World->GetTimerManager().ClearTimer(RepeatTimerHandle);

	{
		TWeakObjectPtr<UDiaGameplayAbility> AbilityPtr = Ability;
		TWeakObjectPtr<UWorld> WorldPtr = World;
		TArray<UDiaSkillVariant*> RepeatVariants = Variants;
		FDiaSkillVariantContext RepeatContext = Context;
		FSkillSpawnRuntime RepeatRuntime = Runtime;

		FTimerDelegate RepeatDelegate;
		RepeatDelegate.BindLambda([this, AbilityPtr, WorldPtr, RepeatVariants, RepeatContext, RepeatRuntime, OnFinished, RemainingRepeatCount]() mutable
			{
				UDiaGameplayAbility* RepeatAbility = AbilityPtr.Get();
				UWorld* RepeatWorld = WorldPtr.Get();
				if (!IsValid(RepeatAbility) || !RepeatWorld)
				{
					OnFinished.ExecuteIfBound();
					return;
				}

				SpawnCast(RepeatVariants, RepeatContext, RepeatAbility, RepeatRuntime);
				--RemainingRepeatCount;

				if (RemainingRepeatCount <= 0)
				{
					RepeatWorld->GetTimerManager().ClearTimer(RepeatTimerHandle);
					OnFinished.ExecuteIfBound();
				}
			});

		World->GetTimerManager().SetTimer(
			RepeatTimerHandle,
			RepeatDelegate,
			FMath::Max(RepeatDelay, 0.01f),
			false);
	}

	return true;
}
