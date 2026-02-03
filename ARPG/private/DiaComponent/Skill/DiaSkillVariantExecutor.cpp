// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/DiaSkillVariantExecutor.h"
#include "DiaComponent/Skill/Effect/DiaVariantEffect_MultipleShot.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"


void UDiaSkillVariantExecutor::InitializeExecutor()
{
	EffectsByTag.Add(FDiaGameplayTags::Get().GASData_MultipleShot, NewObject<UDiaVariantEffect_MultipleShot>(this));
}

void UDiaSkillVariantExecutor::ExecuteEffect(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability)
{
	FSkillSpawnRuntime Runtime;
	ApplyEffects(Variants, Context, Runtime);


}

void UDiaSkillVariantExecutor::ApplyEffects(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, FSkillSpawnRuntime& Runtime)
{
	for (const UDiaSkillVariant* Variant : Variants)
	{
		if (Variant)
		{
			const FDiaSkillVariantSpec Spec = Variant->GetVariantSpec();
			if (UDiaSkillVariantEffect* Effect = EffectsByTag.FindRef(Spec.SkillTag))
			{
				Effect->Apply(Spec, Context, Runtime);
			}
		}
	}
}

void UDiaSkillVariantExecutor::ApplyEffects(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, FSkillHitRuntime& Runtime)
{
	for (const UDiaSkillVariant* Variant : Variants)
	{
		if (Variant)
		{
			const FDiaSkillVariantSpec Spec = Variant->GetVariantSpec();
			if (UDiaSkillVariantEffect* Effect = EffectsByTag.FindRef(Spec.SkillTag))
			{
				Effect->Apply(Spec, Context, Runtime);
			}
		}
	}
}
