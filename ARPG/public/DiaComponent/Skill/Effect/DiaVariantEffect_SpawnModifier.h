// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "DiaVariantEffect_SpawnModifier.generated.h"

/**
 * Spawn 단계에서 적용되는 Variant 효과들을 처리합니다.
 */
UCLASS()
class ARPG_API UDiaVariantEffect_SpawnModifier : public UDiaSkillVariantEffect
{
	GENERATED_BODY()

public:
	virtual void Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime) const override;
};
