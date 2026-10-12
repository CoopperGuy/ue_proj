// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/Exec_Damage.h"
#include "GAS/DiaEquipmentCombat.h"
#include "AbilitySystemComponent.h"

#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"
#include "Logging/ARPGLogChannels.h"

namespace DamageCapture
{
	// 캡처 정의
	struct FStatics
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
		DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);
		DECLARE_ATTRIBUTE_CAPTUREDEF(DamageIncreaseOption);
		DECLARE_ATTRIBUTE_CAPTUREDEF(CritChance);
		DECLARE_ATTRIBUTE_CAPTUREDEF(CritDamage);

		FStatics()
		{
			DEFINE_ATTRIBUTE_CAPTUREDEF(UDiaAttributeSet, AttackPower, Source, true);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UDiaAttributeSet, Defense, Target, true);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UDiaAttributeSet, DamageIncreaseOption, Source, true);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UDiaAttributeSet, CritChance, Source, true);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UDiaAttributeSet, CritDamage, Source, true);
		}
	};
	static const FStatics& Statics()
	{
		static FStatics S;
		return S;
	}
}

UExec_Damage::UExec_Damage()
{
	RelevantAttributesToCapture.Add(DamageCapture::Statics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageCapture::Statics().DefenseDef);
	RelevantAttributesToCapture.Add(DamageCapture::Statics().DamageIncreaseOptionDef);
	RelevantAttributesToCapture.Add(DamageCapture::Statics().CritChanceDef);
	RelevantAttributesToCapture.Add(DamageCapture::Statics().CritDamageDef);
}

void UExec_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	//UExec_Damage 실행을 소유한 GameplayEffectSpec(DiaGameplayEffect_Damage)의 참조
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
    UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
    if (!SourceASC || !TargetASC || SourceASC == TargetASC
        || TargetASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute()) <= 0.f
        || TargetASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Invincible)) return;


	// SetByCaller로 주입 가능한 런타임 값(선택)
    const float DamageBase = Spec.GetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag(FName("GASData.DamageBase")), false, 0.f);
    const float CritMul = Spec.GetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag(FName("GASData.CritMultiplier")), false, 1.f);
	const float DamageMul = Spec.GetSetByCallerMagnitude(
		FDiaGameplayTags::Get().GASData_Variant_DamageMultiplier, false, 1.f);

	// 캡처된 속성 읽기
	FAggregatorEvaluateParameters EvalParams;
	float AttackPower = 0.f, Defense = 0.f, WeaponDamageOption = 0.f, CritChance = 0.f, CritDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().AttackPowerDef, EvalParams, AttackPower);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().DefenseDef, EvalParams, Defense);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().DamageIncreaseOptionDef, EvalParams, WeaponDamageOption);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().CritChanceDef, EvalParams, CritChance);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().CritDamageDef, EvalParams, CritDamage);


	if (Defense <= 0.f) Defense = 1.f; // 방어력이 0이하인 경우 나누기 방지 위해 1로 보정
	CritChance = FMath::Clamp(CritChance, 0.f, 100.f);
	CritDamage = FMath::Max(CritDamage, 100.f);

	const bool bCritical = FMath::FRandRange(0.f, 100.f) < CritChance;
	// The caller supplies a multiplier (1 is neutral); CritDamage is percentage points.
	const float FinalCritMul = bCritical ? CritDamage * .01f * FMath::Max(CritMul, 0.f) : 1.f;
	// 최종 공식(예시)
	const float WeaponDamageMultiplier = WeaponDamageOption * 0.01f;
	const float raw = FMath::Max(0.f, (DamageBase * DamageMul + (AttackPower + AttackPower * WeaponDamageMultiplier) - Defense) / Defense);
	const float finalDamage = FMath::RoundToFloat(raw * FinalCritMul * DiaEquipmentCombat::DamageMultiplier(SourceASC, TargetASC));

	if (finalDamage <= 0.f) return; // 가드

	// Health에 음수로 적용(감소)
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UDiaAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -finalDamage));
}
