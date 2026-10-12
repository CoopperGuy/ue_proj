#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

// Shared by combat, option generation and tooltip text.
namespace DiaEquipmentOptionRules
{
    constexpr float LowHealthRatio = .35f;
    constexpr float HealthyRatio = .80f;
    constexpr float FrostChance = .25f;
    constexpr float FrostDuration = 2.f;
    constexpr float FrostCooldown = 3.f;
    constexpr float LifelineCooldown = 12.f;

    ARPG_API bool IsSpecial(const FGameplayTag& Tag);
    ARPG_API bool IsCombatOption(const FGameplayTag& Tag);
    ARPG_API FString SpecialDescription(const FGameplayTag& Tag, float Value);

    inline bool IsLowHealth(float Health, float Maximum)
    {
        return Health > 0.f && Maximum > 0.f && Health <= Maximum * LowHealthRatio;
    }

    inline float ConfirmedHealthLoss(float Before, float After)
    {
        return FMath::Clamp(Before - FMath::Max(0.f, After), 0.f, FMath::Max(0.f, Before));
    }

    // Cooldowns belong to the wearer and family, so equipment swaps cannot reset them.
    struct FProcCooldowns
    {
        TMap<FGameplayTag, double> ReadyAt;

        bool TryActivate(const FGameplayTag& Tag, double Now, float Chance, float Roll, float Cooldown)
        {
            if (!Tag.IsValid() || Chance <= 0.f || Roll >= FMath::Clamp(Chance, 0.f, 1.f)) return false;
            if (const double* Ready = ReadyAt.Find(Tag); Ready && Now < *Ready) return false;
            ReadyAt.Add(Tag, Now + FMath::Max(0.f, Cooldown));
            return true;
        }
    };
}
