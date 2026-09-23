// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect_Hit.h"

#include "Skill/DiaSkillActor.h"
#include "GAS/DiaGameplayTags.h"
#include "Logging/ARPGLogChannels.h"

void UDiaSkillVariantEffect_Pierce::Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime) const
{
	if (Runtime.GetType() != ESkillVariantRuntimeType::Hit)
	{
		return;
	}

	FSkillHitRuntime& HitRuntime = static_cast<FSkillHitRuntime&>(Runtime);

	// The actor consumes one hit before evaluating variants. This callback runs
	// once per variant, so it must never decrement or refill the hit budget.

	//맞은 후 스킬 복사 관련 내용
	if(Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_Fork))
	{
		for(const auto& SkillActor : Context.SkillActors)
		{
			//복제된 것은 두번 fork 불가능.
			if(SkillActor && !SkillActor->IsSpawnedByFork())
			{
				HitRuntime.ForkCount += static_cast<int32>(Spec.ModifierValue);
				UE_LOG(LogARPG, Log, TEXT("UDiaSkillVariantEffect_Pierce::Apply - Applying Fork Effect, ForkCount: %d"), HitRuntime.ForkCount);
			}
		}
	}
}

void UDiaSkillVariantEffect_Explosion::Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime) const
{
	if (Runtime.GetType() != ESkillVariantRuntimeType::Hit)
	{
		return;
	}

	FSkillHitRuntime& HitRuntime = static_cast<FSkillHitRuntime&>(Runtime);

	if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_ExplodeOnHit))
	{
		HitRuntime.ExplosionRadius = Spec.ModifierValue;

		for(ADiaSkillActor* SkillActor : Context.SkillActors)
		{
			if (SkillActor)
			{
				SkillActor->ExplodeAdditioanlly(HitRuntime.ExplosionRadius);
			}
		}
	}
}

void UDiaSkillVariantEffect_SpawnGround::Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime) const
{
	if (Runtime.GetType() != ESkillVariantRuntimeType::Hit)
	{
		return;
	}
	FSkillHitRuntime& HitRuntime = static_cast<FSkillHitRuntime&>(Runtime);
	if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_BurningGround))
	{
		HitRuntime.ActorParams.SetMagnitude(FDiaGameplayTags::Get().GASData_Variant_BurningGround, Spec.ModifierValue);
	}
}
