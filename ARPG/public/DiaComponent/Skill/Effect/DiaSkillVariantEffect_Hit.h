// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "DiaSkillVariantEffect_Hit.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaSkillVariantEffect_Pierce : public UDiaSkillVariantEffect
{
	GENERATED_BODY()
	
public:
	virtual void Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime) const override;
};

UCLASS()
class ARPG_API UDiaSkillVariantEffect_Explosion : public UDiaSkillVariantEffect
{
	GENERATED_BODY()
	
public:
	virtual void Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime) const override;
};


UCLASS()
class ARPG_API UDiaSkillVariantEffect_SpawnGround : public UDiaSkillVariantEffect
{
	GENERATED_BODY()
	
public:
	virtual void Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime) const override;
};
