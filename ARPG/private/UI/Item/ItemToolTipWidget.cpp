#include "UI/Item/ItemToolTipWidget.h"
#include "System/ItemSubsystem.h"
#include "Types/DiaEquipmentRules.h"
#include "Types/DiaEquipmentOptionRules.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

namespace
{
    FString StatName(const FGameplayTag& Tag)
    {
        const FString Key = Tag.ToString();
        if (Key.EndsWith(TEXT("MaxHealth"))) return TEXT("최대 체력");
        if (Key.EndsWith(TEXT("MaxMana"))) return TEXT("최대 마나");
        if (Key.EndsWith(TEXT("AttackPower"))) return TEXT("공격력");
        if (Key.EndsWith(TEXT("Defense"))) return TEXT("방어력");
        if (Key.EndsWith(TEXT("MovementSpeed"))) return TEXT("이동 속도");
        if (Key.EndsWith(TEXT("CritChance")) || Key.EndsWith(TEXT("CriticalChance"))) return TEXT("치명타 확률");
        if (Key.EndsWith(TEXT("CritDamage"))) return TEXT("치명타 피해");
        if (Key.EndsWith(TEXT("CooldownReduction"))) return TEXT("재사용 대기시간 감소");
        if (Key.EndsWith(TEXT("DamageIncreaseOption"))) return TEXT("공격력 증가");
        if (Key.EndsWith(TEXT("LifeSteal"))) return TEXT("생명력 흡수");
        if (Key.EndsWith(TEXT("GoldFind"))) return TEXT("골드 획득량");
        if (Key.EndsWith(TEXT("Strength"))) return TEXT("힘");
        if (Key.EndsWith(TEXT("Dexterity"))) return TEXT("민첩");
        if (Key.EndsWith(TEXT("Intelligence"))) return TEXT("지능");
        return Key;
    }

    FString FormatStat(const FGameplayTag& Tag, float Value, EItemOptionScalingType Scaling)
    {
        const FString Key = Tag.ToString();
        const bool bPercent = Scaling == EItemOptionScalingType::IOST_Percent
            || Key.EndsWith(TEXT("CritChance")) || Key.EndsWith(TEXT("CritDamage"))
            || Key.EndsWith(TEXT("CooldownReduction")) || Key.EndsWith(TEXT("DamageIncreaseOption"));
        if (Scaling == EItemOptionScalingType::IOST_Multiplier)
            return FString::Printf(TEXT("%s ×%.2f"), *StatName(Tag), Value);
        return FString::Printf(TEXT("%s %+.1f%s"), *StatName(Tag), Value, bPercent ? TEXT("%") : TEXT(""));
    }
}

void UItemToolTipWidget::SetToolTipItem(const FInventorySlot& InItem)
{
    ToolTipItem = InItem;
    UItemSubsystem* Items = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
    const FItemBase* Data = Items ? Items->FindItemData(InItem.ItemInstance) : nullptr;
    if (ItemName)
    {
        ItemName->SetText(Items ? Items->GetItemDisplayName(InItem.ItemInstance) : FText::FromName(InItem.ItemInstance.ItemID));
        ItemName->SetColorAndOpacity(FSlateColor(Data ? DiaEquipmentRules::RarityColor(Data->Rarity) : FLinearColor::White));
    }
    if (ItemDesc)
    {
        FString Text;
        if (Data)
        {
            Text = FString::Printf(TEXT("%s · %s\n아이템 레벨 %d\n\n%s\n"),
                DiaEquipmentRules::RarityName(Data->Rarity), DiaEquipmentRules::SlotName(Data->EquipmentSlot),
                InItem.ItemInstance.Level, *Data->Description.ToString());
        }
        TArray<FGameplayTag> Keys;
        InItem.ItemInstance.ModifiedStats.GetKeys(Keys);
        Keys.Sort([](const FGameplayTag& A, const FGameplayTag& B) { return A.ToString() < B.ToString(); });
        if (!Keys.IsEmpty()) Text += TEXT("\n기본 능력치\n");
        for (const FGameplayTag& Key : Keys)
        {
            const FDiaItemStatOption& Stat = InItem.ItemInstance.ModifiedStats[Key];
            Text += FormatStat(Key, Stat.Values, Stat.ScalingType) + TEXT("\n");
        }
        FString SpecialText;
        auto AppendOptions = [&](const TCHAR* Label, const TArray<FDiaActualItemOption>& Options)
        {
            FString OrdinaryText;
            for (const FDiaActualItemOption& Option : Options)
            {
                const FString Line = FString::Printf(TEXT("%s  [T%d]\n"), *Option.GetOptionDescription().ToString(), Option.TierIndex);
                if (DiaEquipmentOptionRules::IsSpecial(Option.GrantedTag)) SpecialText += Line;
                else OrdinaryText += Line;
            }
            if (!OrdinaryText.IsEmpty()) Text += FString::Printf(TEXT("\n%s\n%s"), Label, *OrdinaryText);
        };
        AppendOptions(TEXT("접두 옵션"), InItem.ItemInstance.PrefixOptions);
        AppendOptions(TEXT("접미 옵션"), InItem.ItemInstance.SuffixOptions);
        if (!SpecialText.IsEmpty()) Text += TEXT("\n특수 옵션\n") + SpecialText + TEXT("같은 특수 옵션은 가장 높은 수치만 적용\n");
        if (Data && Data->bIsEquippable) Text += TEXT("\n우클릭: 착용 / 해제 · 드래그: 이동");
        ItemDesc->SetText(FText::FromString(Text));
        ItemDesc->SetAutoWrapText(true);
        ItemDesc->SetWrapTextAt(320.0f);
    }
    if (ItemIcon)
    {
        ItemIcon->SetBrushFromTexture(nullptr);
        if (Data && Data->IconPath.IsValid())
        {
            TSoftObjectPtr<UTexture2D> Texture(Data->IconPath);
            ItemIcon->SetBrushFromTexture(Texture.LoadSynchronous());
        }
    }
}
