// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Effect/DiaVariantEffect_MultipleShot.h"

void UDiaVariantEffect_MultipleShot::Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillSpawnRuntime& Runtime) const
{
	Runtime.ExtraSpawnCount += 1;
}

