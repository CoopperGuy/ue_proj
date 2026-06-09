// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaSkillVariantEffect.generated.h"


enum class ESkillVariantRuntimeType : uint8
{
	None,
	Spawn,
	Hit,
	Modi
};

USTRUCT()
struct FDiaSkillRuntimeParams
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<FGameplayTag, float> Magnitudes;

	float GetMagnitude(const FGameplayTag& Tag, float DefaultValue = 0.f) const
	{
		if (const float* Value = Magnitudes.Find(Tag))
		{
			return *Value;
		}

		return DefaultValue;
	}

	int32 GetInt(const FGameplayTag& Tag, int32 DefaultValue = 0) const
	{
		return FMath::RoundToInt(GetMagnitude(Tag, static_cast<float>(DefaultValue)));
	}

	void SetMagnitude(const FGameplayTag& Tag, float Value)
	{
		Magnitudes.FindOrAdd(Tag) = Value;
	}

	void AddMagnitude(const FGameplayTag& Tag, float Value)
	{
		Magnitudes.FindOrAdd(Tag) += Value;
	}

	void MultiplyMagnitude(const FGameplayTag& Tag, float Value, float DefaultValue = 1.f)
	{
		if (float* Magnitude = Magnitudes.Find(Tag))
		{
			*Magnitude *= Value;
			return;
		}

		Magnitudes.Add(Tag, DefaultValue * Value);
	}
};

USTRUCT()
struct FSkillVariantRuntime
{
	GENERATED_BODY()

	FSkillVariantRuntime() = default;
	explicit FSkillVariantRuntime(ESkillVariantRuntimeType InType)
		: Type(InType)
	{
	}

	ESkillVariantRuntimeType GetType() const { return Type; }

private:
	ESkillVariantRuntimeType Type = ESkillVariantRuntimeType::None;
};

USTRUCT()
struct FSkillSpawnRuntime : public FSkillVariantRuntime
{
	GENERATED_BODY()

	FSkillSpawnRuntime()
		: FSkillVariantRuntime(ESkillVariantRuntimeType::Spawn)
	{
	}

	int32 ExtraSpawnCount = 0;
	float AngleOffset = 0.f;
	float BigSpawnChance = 0.f;
	FVector CloseRangeBonus = FVector::ZeroVector;
	FDiaSkillRuntimeParams ActorParams;
	// 필요한 런타임 상태 확장 가능
};

USTRUCT()
struct FSkillHitRuntime : public FSkillVariantRuntime
{
	GENERATED_BODY()

	FSkillHitRuntime()
		: FSkillVariantRuntime(ESkillVariantRuntimeType::Hit)
	{
	}

	int32 PierceCount = 0;
	float ExplosionRadius = 0;
	int32 ForkCount = 0;
	float FreezeBuildUpAmount = 0.f;
	float LifestealPercent = 0.f;
	float KnockbackStrength = 0.f;
	float VortexStrength = 0.f;
	FDiaSkillRuntimeParams ActorParams;
	// 히트 시 필요한 런타임 상태 확장 가능
};

USTRUCT()
struct FSkillModifierRuntime : public FSkillVariantRuntime
{
	GENERATED_BODY()

	FSkillModifierRuntime()
		: FSkillVariantRuntime(ESkillVariantRuntimeType::Modi)
	{
	}

	//쿨	다운 감소 퍼센트
	float CDRP = 1.f;
	//마나 감소 퍼센트
	float MCRP = 1.f;
	// 히트 시 필요한 런타임 상태 확장 가능
};


/**
 * 
 */
UCLASS(Abstract)
class ARPG_API UDiaSkillVariantEffect : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Apply(
		const FDiaSkillVariantSpec& Spec,
		FDiaSkillVariantContext& Context,
		FSkillVariantRuntime& Runtime) const;

};
