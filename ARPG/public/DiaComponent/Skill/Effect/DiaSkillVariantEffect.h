// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaSkillVariantEffect.generated.h"


USTRUCT()
struct FSkillSpawnRuntime
{
	GENERATED_BODY()

	int32 ExtraSpawnCount = 0;
	float AngleOffset = 0.f;
	// 필요한 런타임 상태 확장 가능
};

USTRUCT()
struct FSkillHitRuntime
{
	GENERATED_BODY()

	int32 PierceCount = 0;
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
		struct FSkillSpawnRuntime& Runtime) const;

	virtual void Apply(
		const FDiaSkillVariantSpec& Spec,
		FDiaSkillVariantContext& Context,
		struct FSkillHitRuntime& Runtime) const;

};
