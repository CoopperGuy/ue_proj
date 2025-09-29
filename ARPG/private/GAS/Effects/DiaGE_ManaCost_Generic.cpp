// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/DiaGE_ManaCost_Generic.h"
#include "GAS/DiaAttributeSet.h"
UDiaGE_ManaCost_Generic::UDiaGE_ManaCost_Generic()
{
    // Instant effect (한번만 적용)
    DurationPolicy = EGameplayEffectDurationType::Instant;

    // Health 속성에 대미지 적용
    FGameplayModifierInfo DamageModifier;
    DamageModifier.ModifierMagnitude = FScalableFloat(-50.0f); // -50 데미지 (기본값)
    DamageModifier.ModifierOp = EGameplayModOp::Additive;
    DamageModifier.Attribute = UDiaAttributeSet::GetManaAttribute();

    Modifiers.Add(DamageModifier);
}
