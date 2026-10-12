#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UAbilitySystemComponent;

namespace DiaEquipmentCombat
{
    // Only active equipment effects contribute. Special families use the strongest roll.
    ARPG_API float Magnitude(UAbilitySystemComponent* ASC, const FGameplayTag& Tag);
    ARPG_API float DamageMultiplier(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target);
    ARPG_API void OnConfirmedHit(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target,
        float ActualDamage, bool bKilled);
}
