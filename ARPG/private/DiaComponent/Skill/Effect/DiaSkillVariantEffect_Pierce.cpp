// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect_Pierce.h"

void UDiaSkillVariantEffect_Pierce::Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillHitRuntime& Runtime) const
{
	// PierceCount가 0이면 초기값 설정 (ModifierValue는 관통 횟수)
	if (Runtime.PierceCount == 0)
	{
		Runtime.PierceCount = FMath::Max(1, static_cast<int32>(Spec.ModifierValue));
	}
	
	// 히트 시 PierceCount 감소
	Runtime.PierceCount -= 1;
}
