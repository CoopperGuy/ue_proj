// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	float DamageMultiplier = 1.f;
	int32 PierceCount = 0;
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
