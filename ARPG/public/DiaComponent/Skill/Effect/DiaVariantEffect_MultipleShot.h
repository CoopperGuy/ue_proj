// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "DiaVariantEffect_MultipleShot.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaVariantEffect_MultipleShot : public UDiaSkillVariantEffect
{
	GENERATED_BODY()
public:
	virtual void Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillSpawnRuntime& Runtime) const override;
};
