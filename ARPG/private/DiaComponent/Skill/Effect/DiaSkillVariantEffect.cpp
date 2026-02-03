// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"

void UDiaSkillVariantEffect::Apply(
	const FDiaSkillVariantSpec& Spec,
	FDiaSkillVariantContext& Context,
	FSkillSpawnRuntime& Runtime) const
{
	// 기본 구현: 자식 클래스에서 오버라이드해야 함
}

void UDiaSkillVariantEffect::Apply(
	const FDiaSkillVariantSpec& Spec,
	FDiaSkillVariantContext& Context,
	FSkillHitRuntime& Runtime) const
{
	// 기본 구현: 자식 클래스에서 오버라이드해야 함
}
