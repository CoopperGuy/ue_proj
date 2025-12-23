// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/Exec_Damage.h"
#include "GAS/Effects/DiaGE_OptionGeneric.h"

#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"

namespace DamageCapture
{
	// 캡처 정의
	struct FStatics
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
		DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);
		DECLARE_ATTRIBUTE_CAPTUREDEF(DamageIncreaseOption);

		FStatics()
		{
			DEFINE_ATTRIBUTE_CAPTUREDEF(UDiaAttributeSet, AttackPower, Source, true);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UDiaAttributeSet, Defense, Target, true);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UDiaAttributeSet, DamageIncreaseOption, Source, true);
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
}

void UExec_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, 
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	//UExec_Damage 실행을 소유한 GameplayEffectSpec(DiaGameplayEffect_Damage)의 참조
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// SetByCaller로 주입 가능한 런타임 값(선택)
    const float DamageBase = Spec.GetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag(FName("GASData.DamageBase")), false, 0.f);
    const float CritMul = Spec.GetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag(FName("GASData.CritMultiplier")), false, 1.f);

	// 캡처된 속성 읽기
	FAggregatorEvaluateParameters EvalParams;
	float AttackPower = 0.f, Defense = 0.f, WeaponDamageOption = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().AttackPowerDef, EvalParams, AttackPower);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().DefenseDef, EvalParams, Defense);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageCapture::Statics().DamageIncreaseOptionDef, EvalParams, WeaponDamageOption);

	// 최종 공식(예시)
	const float raw = FMath::Max(0.f, DamageBase + (AttackPower + AttackPower * WeaponDamageOption) - Defense);
	const float finalDamage = FMath::RoundToFloat(raw * CritMul);
	if (finalDamage <= 0.f) return; // 가드

	UE_LOG(LogTemp, Warning, TEXT("Damage Exec: Base=%f, Atk=%f, Def=%f, Option=%f, CritMul=%f => Damage=%f"),
		DamageBase, AttackPower, Defense, WeaponDamageOption, CritMul, finalDamage);
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	if (IsValid(SourceASC))
	{
		FGameplayEffectQuery Query;
		Query.EffectDefinition = UDiaGE_OptionGeneric::StaticClass();
		TArray<FActiveGameplayEffectHandle> Handles = SourceASC->GetActiveEffects(Query);

		FGameplayTagContainer SearchTags;
		SearchTags.AddTag(FDiaGameplayTags::Get().ItemOptionLifeSteal);  // Effect에 붙인 태그
		TArray<FActiveGameplayEffectHandle> TestHandles = SourceASC->GetActiveEffectsWithAllTags(SearchTags);

		for(FActiveGameplayEffectHandle& Handle : Handles)
		{
			const FActiveGameplayEffect* ActiveGE = SourceASC->GetActiveGameplayEffect(Handle);
			if (ActiveGE)
			{
				//흡혈 체크
				float LifeSteel = ActiveGE->Spec.GetSetByCallerMagnitude(FDiaGameplayTags::Get().ItemOptionLifeSteal);
				if (LifeSteel > 0.f)
				{
					SourceASC->ApplyModToAttribute(
						UDiaAttributeSet::GetHealthAttribute(),
						EGameplayModOp::Additive,
						finalDamage * LifeSteel * 0.01f);
				}
			}
		}
	}

	// Health에 음수로 적용(감소)
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UDiaAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -finalDamage));
}
