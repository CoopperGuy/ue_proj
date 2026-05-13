// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "DiaVariantEffect_ActiveModifier.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaVariantEffect_ActiveModifier : public UDiaSkillVariantEffect
{
	GENERATED_BODY()
	
public:
	virtual void Apply(
		const FDiaSkillVariantSpec& Spec,
		FDiaSkillVariantContext& Context,
		FSkillVariantRuntime& Runtime) const;	
};
