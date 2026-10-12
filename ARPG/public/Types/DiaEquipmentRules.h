#pragma once

#include "Types/ItemBase.h"

namespace DiaEquipmentRules
{
    // Prefix/suffix limits. Explicit debug options use these same limits.
    inline FIntPoint OptionBudget(EItemRarity Rarity)
    {
        switch (Rarity)
        {
        case EItemRarity::EIR_Uncommon: return FIntPoint(1, 1);
        case EItemRarity::EIR_Rare: return FIntPoint(2, 1);
        case EItemRarity::EIR_Epic: return FIntPoint(2, 2);
        case EItemRarity::EIR_Legendary: return FIntPoint(3, 2);
        case EItemRarity::EIR_Mythic: return FIntPoint(3, 3);
        default: return FIntPoint::ZeroValue;
        }
    }

    inline FLinearColor RarityColor(EItemRarity Rarity)
    {
        switch (Rarity)
        {
        case EItemRarity::EIR_Uncommon: return FLinearColor(0.25f, 0.8f, 0.42f);
        case EItemRarity::EIR_Rare: return FLinearColor(0.22f, 0.52f, 1.0f);
        case EItemRarity::EIR_Epic: return FLinearColor(0.7f, 0.32f, 1.0f);
        case EItemRarity::EIR_Legendary: return FLinearColor(1.0f, 0.55f, 0.12f);
        case EItemRarity::EIR_Mythic: return FLinearColor(1.0f, 0.22f, 0.3f);
        default: return FLinearColor(0.65f, 0.7f, 0.75f);
        }
    }

    inline const TCHAR* RarityName(EItemRarity Rarity)
    {
        switch (Rarity)
        {
        case EItemRarity::EIR_Uncommon: return TEXT("고급");
        case EItemRarity::EIR_Rare: return TEXT("희귀");
        case EItemRarity::EIR_Epic: return TEXT("영웅");
        case EItemRarity::EIR_Legendary: return TEXT("전설");
        case EItemRarity::EIR_Mythic: return TEXT("신화");
        default: return TEXT("일반");
        }
    }

    inline const TCHAR* SlotName(EEquipmentSlot Slot)
    {
        switch (Slot)
        {
        case EEquipmentSlot::EES_Head: return TEXT("투구");
        case EEquipmentSlot::EES_Chest: return TEXT("갑옷");
        case EEquipmentSlot::EES_Belt: return TEXT("허리띠");
        case EEquipmentSlot::EES_Feet: return TEXT("장화");
        case EEquipmentSlot::EES_Hands: return TEXT("장갑");
        case EEquipmentSlot::EES_Weapon: return TEXT("무기");
        case EEquipmentSlot::EES_Shield: return TEXT("방패");
        case EEquipmentSlot::EES_RingL:
        case EEquipmentSlot::EES_RingR: return TEXT("반지 · 양손 착용 가능");
        case EEquipmentSlot::EES_Amulet: return TEXT("목걸이");
        default: return TEXT("아이템");
        }
    }
}
