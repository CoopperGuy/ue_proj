// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Effect/DiaVariantEffect_SpawnModifier.h"
#include "GAS/DiaGameplayTags.h"

void UDiaVariantEffect_SpawnModifier::Apply(const FDiaSkillVariantSpec& Spec, FDiaSkillVariantContext& Context, FSkillVariantRuntime& Runtime) const
{
	if (Runtime.GetType() != ESkillVariantRuntimeType::Spawn)
	{
		return;
	}

	FSkillSpawnRuntime& SpawnRuntime = static_cast<FSkillSpawnRuntime&>(Runtime);

	if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_MultipleShot))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Spawn Modifier Effect"));
		SpawnRuntime.ExtraSpawnCount += Spec.ModifierValue;
	}
	else if(Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_DurationMultiplier))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Duration Multiplier Effect"));
	}else if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_RepeatCast))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Repeat Cast Effect"));
	}
	else if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_FreezeStack))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Freeze Stack Effect"));
	}
	else if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_BurningGround))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Burning Ground Effect"));
	}else if(Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_LifeSteal))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Life Steal Effect"));
	}
	else if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_Knockback))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Knockback Effect"));
	}
	else if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_BigProjectile))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Big Projectile Effect"));
	}
	else if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Variant_DamageMultiplier))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Damage Multiplier Effect"));
		SpawnRuntime.DamageMultiplier *= Spec.ModifierValue;
	}
	else if (Spec.SkillTag.MatchesTagExact(FDiaGameplayTags::Get().GASData_Pierce))
	{
		UE_LOG(LogTemp, Log, TEXT("UDiaVariantEffect_SpawnModifier::Apply - Applying Pierce Effect"));
		SpawnRuntime.PierceCount += static_cast<int32>(Spec.ModifierValue);
	}

}
