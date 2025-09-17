// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/Exec_Damage.h"
#include "GAS/DiaAttributeSet.h"

namespace DamageCapture
{
	// 캡처 정의
	struct FStatics
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
		DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);

		FStatics()
		{
			DEFINE_ATTRIBUTE_CAPTUREDEF(UARPGAttributeSet, AttackPower, Source, true);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UARPGAttributeSet, Defense, Target, true);
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
}

void UExec_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// SetByCaller로 주입 가능한 런타임 값(선택)
	const float DamageBase = Spec.GetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.DamageBase")), false, 0.f);
	const float CritMul = Spec.GetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag(FName("Data.CritMultiplier")), false, 1.f);

	// 캡처된 속성 읽기
	FAggregatorEvaluateParameters EvalParams;
	float AttackPower = 0.f, Defense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().AttackPowerDef, EvalParams, AttackPower);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().DefenseDef, EvalParams, Defense);

	// 최종 공식(예시)
	const float raw = FMath::Max(0.f, DamageBase + AttackPower - Defense);
	const float finalDamage = FMath::RoundToFloat(raw * CritMul);
	if (finalDamage <= 0.f) return; // 가드

	// Health에 음수로 적용(감소)
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UDiaAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -finalDamage));
}
