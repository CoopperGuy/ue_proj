#include "Types/DiaEquipmentOptionRules.h"
#include "Types/DiaItemOptionRow.h"
#include "GAS/DiaGameplayTags.h"

bool DiaEquipmentOptionRules::IsSpecial(const FGameplayTag& Tag)
{
    using namespace DiaGameplayTags;
    return Tag == ItemOption_Execution || Tag == ItemOption_Ambush || Tag == ItemOption_Berserker
        || Tag == ItemOption_Guardian || Tag == ItemOption_Frostbound || Tag == ItemOption_Lifeline;
}

bool DiaEquipmentOptionRules::IsCombatOption(const FGameplayTag& Tag)
{
    using namespace DiaGameplayTags;
    return IsSpecial(Tag) || Tag == ItemOption_LifeSteal || Tag == ItemOption_HealthOnKill || Tag == ItemOption_ManaOnKill;
}

FString DiaEquipmentOptionRules::SpecialDescription(const FGameplayTag& Tag, float Value)
{
    using namespace DiaGameplayTags;
    if (Tag == ItemOption_Execution)
        return FString::Printf(TEXT("처형 — 체력 %.0f%% 이하인 적에게 주는 피해 +%.1f%%"), LowHealthRatio * 100.f, Value);
    if (Tag == ItemOption_Ambush)
        return FString::Printf(TEXT("선제 공격 — 체력 %.0f%% 이상인 적에게 주는 피해 +%.1f%%"), HealthyRatio * 100.f, Value);
    if (Tag == ItemOption_Berserker)
        return FString::Printf(TEXT("광전사 — 내 체력이 %.0f%% 이하일 때 주는 피해 +%.1f%%"), LowHealthRatio * 100.f, Value);
    if (Tag == ItemOption_Guardian)
        return FString::Printf(TEXT("불굴 — 내 체력이 %.0f%% 이하일 때 받는 피해 %.1f%% 감소"), LowHealthRatio * 100.f, Value);
    if (Tag == ItemOption_Frostbound)
        return FString::Printf(TEXT("서리 손길 — 적중 시 %.0f%% 확률로 %.0f초간 이동 속도 %.1f%% 감소 (재발동 %.0f초)"),
            FrostChance * 100.f, FrostDuration, Value, FrostCooldown);
    if (Tag == ItemOption_Lifeline)
        return FString::Printf(TEXT("생명의 보루 — 피격 후 생존한 상태에서 체력이 %.0f%% 이하이면 최대 체력의 %.1f%% 회복 (재발동 %.0f초)"),
            LowHealthRatio * 100.f, Value, LifelineCooldown);
    return FString();
}

FText FDiaActualItemOption::GetOptionDescription() const
{
    const FString Special = DiaEquipmentOptionRules::SpecialDescription(GrantedTag, Value);
    if (!Special.IsEmpty()) return FText::FromString(Special);
    FString ValueText;
    switch (ScalingType)
    {
    case EItemOptionScalingType::IOST_Percent: ValueText = FString::Printf(TEXT("%+.1f%%"), Value); break;
    case EItemOptionScalingType::IOST_Multiplier: ValueText = FString::Printf(TEXT("×%.2f"), Value); break;
    default: ValueText = FString::Printf(TEXT("%+.1f"), Value); break;
    }
    return FText::FromString(FString::Printf(TEXT("%s %s"), *DisplayName.ToString(), *ValueText));
}
