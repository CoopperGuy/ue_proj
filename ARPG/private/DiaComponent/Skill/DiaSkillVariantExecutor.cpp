// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/DiaSkillVariantExecutor.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"
#include "Logging/ARPGLogChannels.h"


UDiaSkillVariantExecutor::UDiaSkillVariantExecutor()
{
	UE_LOG(LogARPG, Log, TEXT("UDiaSkillVariantExecutor::UDiaSkillVariantExecutor - Constructor Called"));
}

void UDiaSkillVariantExecutor::InitializeExecutor()
{
	UE_LOG(LogARPG, Log, TEXT("UDiaSkillVariantExecutor::InitializeExecutor - Initializing Executor"));
}

//기본적인 형태
void UDiaSkillVariantExecutor::ExecuteEffect(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability)
{
	FSkillSpawnRuntime Runtime;
	ApplyEffects(Variants, Context, Runtime);
}


void UDiaSkillVariantExecutor::ExecuteEffect(const TArray<class UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, const UDiaGameplayAbility* Ability,FSkillVariantRuntime& OutRuntime)
{
	ApplyEffects(Variants, Context, OutRuntime);
}

//이게 자식클래스에서 호출되는것
void UDiaSkillVariantExecutor::ApplyEffects(const TArray<UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime)
{
	for (const UDiaSkillVariant* Variant : Variants)
	{
		if (Variant)
		{
			const FDiaSkillVariantSpec Spec = Variant->GetVariantSpec();
			UE_LOG(LogARPG, Warning, TEXT("UDiaSkillVariantExecutor::ApplyEffects - Applying Effect with Tag: %s"), *Spec.SkillTag.ToString());
			for (const TObjectPtr<UDiaSkillVariantEffect>& Effect : RegisteredEffects)
			{
				Effect->Apply(Spec, Context, Runtime);
			}
		}
	}
}
