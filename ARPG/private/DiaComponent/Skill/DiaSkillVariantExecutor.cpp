// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/DiaSkillVariantExecutor.h"
#include "DiaComponent/Skill/Effect/DiaVariantEffect_MultipleShot.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"


UDiaSkillVariantExecutor::UDiaSkillVariantExecutor()
{
	UE_LOG(LogTemp, Log, TEXT("UDiaSkillVariantExecutor::UDiaSkillVariantExecutor - Constructor Called"));
}

void UDiaSkillVariantExecutor::InitializeExecutor()
{
	UE_LOG(LogTemp, Log, TEXT("UDiaSkillVariantExecutor::InitializeExecutor - Initializing Executor"));
}

//기본적인 형태
void UDiaSkillVariantExecutor::ExecuteEffect(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability)
{
	FSkillSpawnRuntime Runtime;
	ApplyEffects(Variants, Context, Runtime);
}

//이게 자식클래스에서 호출되는것
void UDiaSkillVariantExecutor::ApplyEffects(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, FSkillSpawnRuntime& Runtime)
{
	for (const UDiaSkillVariant* Variant : Variants)
	{
		if (Variant)
		{
			const FDiaSkillVariantSpec Spec = Variant->GetVariantSpec();
			UE_LOG(LogTemp, Warning, TEXT("UDiaSkillVariantExecutor::ApplyEffects - Applying Effect with Tag: %s"), *Spec.SkillTag.ToString());
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
