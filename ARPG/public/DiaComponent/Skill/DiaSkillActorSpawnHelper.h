// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "GameplayEffectTypes.h"
#include "Types/DiaGASSkillData.h"

class ADiaSkillActor;
class UAbilitySystemComponent;
class UDiaGameplayAbility;
class UGameplayEffect;

struct ARPG_API FSpawnLocationData
{
	FVector CharacterLocation = FVector::ZeroVector;
	FVector Direction = FVector::ForwardVector;
	FTransform BaseSpawnTransform = FTransform::Identity;
	FRotator NextProjectileRotator = FRotator::ZeroRotator;
	bool bHasSpawnTransform = false;
};

struct ARPG_API FDiaSkillActorSpawnRequest
{
	UWorld* World = nullptr;
	TSubclassOf<ADiaSkillActor> SkillActorClass = nullptr;
	FTransform SpawnTransform = FTransform::Identity;
	AActor* OwnerActor = nullptr;
	APawn* Instigator = nullptr;
	const FGASSkillData* SkillData = nullptr;
	UAbilitySystemComponent* SourceASC = nullptr;
	TSubclassOf<UGameplayEffect> DamageEffectClass = nullptr;
	UDiaGameplayAbility* OwningAbility = nullptr;
	TArray<FGameplayEffectSpecHandle> TargetEffectSpecs;
	TArray<AActor*> IgnoredActors;
	FDiaSkillRuntimeParams RuntimeParams;
	bool bSpawnedByFork = false;
	bool bLaunch = true;
	bool bApplyAbilityRemovalTimer = true;
};

class ARPG_API FDiaSkillActorSpawnHelper
{
public:
	static FSpawnLocationData CalculateSpawnLocationData(
		const FDiaSkillVariantContext& Context,
		const AActor* Character,
		const FSkillSpawnRuntime& Runtime);

	static FTransform MakeSpawnTransform(
		const FSpawnLocationData& SpawnLocationData,
		const FVector& Direction);

	static void ApplyVariantEffects(
		const TArray<UDiaSkillVariant*>& Variants,
		FDiaSkillVariantContext& Context,
		const UDiaGameplayAbility* Ability,
		FSkillSpawnRuntime& Runtime,
		TArray<FGameplayEffectSpecHandle>& TargetEffectSpecs);

	static ADiaSkillActor* SpawnSkillActor(const FDiaSkillActorSpawnRequest& Request);
};
