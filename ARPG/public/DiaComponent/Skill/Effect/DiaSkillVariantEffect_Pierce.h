// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "DiaSkillVariantEffect_Pierce.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaSkillVariantEffect_Pierce : public UDiaSkillVariantEffect
{
	GENERATED_BODY()
	
public:
	void Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillHitRuntime& Runtime) const;
};
