#include "GAS/Effects/DiaGameplayEffect_Damage.h"
#include "GAS/DiaAttributeSet.h"
#include "GameplayEffect.h"

UDiaGameplayEffect_Damage::UDiaGameplayEffect_Damage()
{
    // Instant effect (한번만 적용)
    DurationPolicy = EGameplayEffectDurationType::Instant;
    
    // Health 속성에 대미지 적용
    FGameplayModifierInfo DamageModifier;
    DamageModifier.ModifierMagnitude = FScalableFloat(-50.0f); // -50 데미지 (기본값)
    DamageModifier.ModifierOp = EGameplayModOp::Additive;
    DamageModifier.Attribute = UDiaAttributeSet::GetHealthAttribute();
    
    Modifiers.Add(DamageModifier);
}