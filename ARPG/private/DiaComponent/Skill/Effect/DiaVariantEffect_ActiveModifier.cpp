// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Effect/DiaVariantEffect_ActiveModifier.h"
#include "GAS/DiaGameplayTags.h"

void UDiaVariantEffect_ActiveModifier::Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime) const
{
	if (Runtime.GetType() != ESkillVariantRuntimeType::Modi)
	{
		return;
	}

	FSkillModifierRuntime& ModiRuntime = static_cast<FSkillModifierRuntime&>(Runtime);
	if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_CooldownReduction))
	{
		ModiRuntime.CDRP *= Spec.ModifierValue;
	}
	else if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_ManaCostMultiplier))
	{
		ModiRuntime.MCRP *= Spec.ModifierValue;
	}
}


