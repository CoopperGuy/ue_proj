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

void UDiaSkillVariantSpawnExecutor::InitializeExecutor()
{
	RegisteredEffects.Add(NewObject<UDiaVariantEffect_SpawnModifier>(this));
}

void UDiaSkillVariantSpawnExecutor::ExecuteEffect(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability)
{
	if(Context.SkillActorClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - SkillActorClass is null"));
		return;
	}

	FSkillSpawnRuntime Runtime;
	Runtime.ExtraSpawnCount = 1;
	ApplyEffects(Variants, Context, Runtime);	
	BeforeSpawn(Variants, Context, Ability, Runtime);
	AfterSpawn(Variants, Context, Ability, Runtime);
}

void UDiaSkillVariantSpawnExecutor::BeforeSpawn(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability, FSkillSpawnRuntime& Runtime) const
{
	const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
	AActor* Character = (ActorInfo.AvatarActor.Get());
	APawn* Pawn = Cast<APawn>(Character);
	FSpawnLocationData SpawnLocationData = FDiaSkillActorSpawnHelper::CalculateSpawnLocationData(Context, Character, Runtime);
	FVector Direction = SpawnLocationData.Direction;

	UWorld* World = Ability ? Ability->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - World가 유효하지 않습니다."));
		return;
	}

	for (int32 i = 0; i < Runtime.ExtraSpawnCount; i++)
	{
		const FTransform SpawnTransform = FDiaSkillActorSpawnHelper::MakeSpawnTransform(SpawnLocationData, Direction);

		UE_LOG(LogTemp, Log, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - SkillID: %d, Class: %s, SpawnLocation: %s, Direction: %s"),
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
		SpawnRequest.DamageMultiplier = Runtime.DamageMultiplier;
		SpawnRequest.PierceCount = Runtime.PierceCount;

		if (ADiaSkillActor* SpawnedActor = FDiaSkillActorSpawnHelper::SpawnSkillActor(SpawnRequest))
		{
			Context.SkillActors.Add(SpawnedActor);
		}
		// 다음 발사체를 위한 방향 업데이트 (다음 루프에서 사용)
		Direction = SpawnLocationData.NextProjectileRotator.RotateVector(Direction).GetSafeNormal();
	}
}

void UDiaSkillVariantSpawnExecutor::AfterSpawn(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability, FSkillSpawnRuntime& Runtime) const
{



}
