// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/DiaGE_ManaCost_Generic.h"
#include "GAS/DiaAttributeSet.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"
UDiaGE_ManaCost_Generic::UDiaGE_ManaCost_Generic()
{
    // Instant effect (한번만 적용)
    DurationPolicy = EGameplayEffectDurationType::Instant;

    // SetByCaller를 사용하여 동적으로 마나 소모량 설정
    FSetByCallerFloat SetByCallerMagnitude;
    SetByCallerMagnitude.DataTag = FGameplayTag::RequestGameplayTag(FName("GASData.ManaCost"));

    // Modifier 설정: Mana 속성에 적용
    FGameplayModifierInfo ManaCostModifier;
    ManaCostModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);
    ManaCostModifier.ModifierOp = EGameplayModOp::Additive;
    ManaCostModifier.Attribute = UDiaAttributeSet::GetManaAttribute();

    Modifiers.Add(ManaCostModifier);
}
