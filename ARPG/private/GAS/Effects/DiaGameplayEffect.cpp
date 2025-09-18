#include "GAS/Effects/DiaGameplayEffect.h"
#include "GAS/DiaAttributeSet.h"

UDiaGameplayEffect::UDiaGameplayEffect()
{
	// Set default duration policy
	DurationPolicy = EGameplayEffectDurationType::Instant;
}