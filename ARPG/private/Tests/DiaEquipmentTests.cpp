#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "Subsystems/SubsystemCollection.h"
#include "System/ItemSubsystem.h"
#include "DiaComponent/DiaOptionManagerComponent.h"
#include "GAS/DiaGameplayTags.h"
#include "Types/DiaEquipmentRules.h"
#include "Types/DiaEquipmentOptionRules.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaEquipmentBaseStatsTest, "ARPG.Equipment.LevelAndQuantity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDiaEquipmentBaseStatsTest::RunTest(const FString& Parameters)
{
    FItemBase Base;
    Base.ItemID = TEXT("TestSword");
    FDiaItemStatOption Attack;
    Attack.StatTag = DiaGameplayTags::AttributeSet_AttackPower.GetTag();
    Attack.Values = 10.f;
    Base.StatOptions.Add(Attack);
    FDiaItemStatOption Crit;
    Crit.StatTag = DiaGameplayTags::AttributeSet_CritChance.GetTag();
    Crit.Values = 3.f;
    Base.StatOptions.Add(Crit);
    FItemInstance Item = FItemInstance::FromBase(Base);
    Item.Level = 26;
    Item.Quantity = 99;
    Item.RebuildBaseStats(Base);
    TestEqual(TEXT("Level 26 doubles flat base stats without quantity bonus"), Item.ModifiedStats[Attack.StatTag].Values, 20.f);
    TestEqual(TEXT("Percent-point attributes do not grow with level"), Item.ModifiedStats[Crit.StatTag].Values, 3.f);
    const FGuid Identity = Item.InstanceID;
    FInventorySlot Slot;
    Slot.ItemInstance = Item;
    const FInventorySlot RoundTrip = Slot.ToEquippItem().ToInventorySlot(2, 3);
    TestEqual(TEXT("Inventory/equipment transfer keeps identity"), RoundTrip.ItemInstance.InstanceID, Identity);
    TestEqual(TEXT("Transfer keeps rolled stats"), RoundTrip.ItemInstance.ModifiedStats[Attack.StatTag].Values, 20.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaEquipmentOptionRemovalTest, "ARPG.Equipment.SharedOptionRemoval",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDiaEquipmentOptionRemovalTest::RunTest(const FString& Parameters)
{
    UDiaOptionManagerComponent* Options = NewObject<UDiaOptionManagerComponent>();
    FDiaActualItemOption First;
    First.OptionID = TEXT("Fortunate");
    First.GrantedTag = DiaGameplayTags::ItemOption_GoldFind.GetTag();
    First.Value = 8.f;
    FDiaActualItemOption Second = First;
    Second.Value = 12.f;
    Options->AddOption(First);
    Options->AddOption(Second);
    TestEqual(TEXT("Two rings both contribute"), Options->GetTotalOptionMagnitudeByTag(First.GrantedTag), 20.f);
    Options->RemoveOption(First);
    TestEqual(TEXT("Removing one ring preserves the other"), Options->GetTotalOptionMagnitudeByTag(First.GrantedTag), 12.f);
    Options->AddOption(Second);
    Options->RemoveOption(Second);
    TestEqual(TEXT("Identical rolls are removed one at a time"), Options->GetTotalOptionMagnitudeByTag(First.GrantedTag), 12.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaEquipmentRollTest, "ARPG.Equipment.CatalogRolls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDiaEquipmentRollTest::RunTest(const FString& Parameters)
{
    UGameInstance* Game = NewObject<UGameInstance>();
    UItemSubsystem* Items = NewObject<UItemSubsystem>(Game);
    FSubsystemCollection<UGameInstanceSubsystem> Collection;
    Items->Initialize(Collection);
    const TArray<FName> IDs = Items->GetRewardEquipmentIDs();
    TestTrue(TEXT("Catalog contains the equipment pack"), IDs.Num() >= 24);
    for (FName ID : IDs)
    {
        const FItemBase& Base = Items->GetItemData(ID);
        const FIntPoint Budget = DiaEquipmentRules::OptionBudget(Base.Rarity);
        for (int32 Level : { 1, 19, 20, 49, 50, 100 })
        {
            for (int32 Roll = 0; Roll < 25; ++Roll)
            {
                FInventorySlot Item;
                Items->CreateInventoryInstance(Item, ID, Level, true, 10);
                TestEqual(TEXT("Equipment never stacks"), Item.ItemInstance.Quantity, 1);
                TestTrue(TEXT("Prefix budget"), Item.ItemInstance.PrefixOptions.Num() <= Budget.X);
                TestTrue(TEXT("Suffix budget"), Item.ItemInstance.SuffixOptions.Num() <= Budget.Y);
                TSet<FName> Families;
                int32 SpecialCount = 0;
                auto Verify = [&](const TArray<FDiaActualItemOption>& Options)
                {
                    for (const FDiaActualItemOption& Option : Options)
                    {
                        TestFalse(TEXT("No repeated family on one item"), Families.Contains(Option.OptionID));
                        Families.Add(Option.OptionID);
                        if (DiaEquipmentOptionRules::IsSpecial(Option.GrantedTag)) ++SpecialCount;
                        TestEqual(TEXT("Tier follows item level"), Option.TierIndex, Level < 20 ? 3 : Level < 50 ? 2 : 1);
                        TestTrue(TEXT("Gameplay-backed option"), FDiaGameplayTags::Get().GetItemOptionList().Contains(Option.GrantedTag));
                    }
                };
                Verify(Item.ItemInstance.PrefixOptions);
                Verify(Item.ItemInstance.SuffixOptions);
                TestEqual(TEXT("Epic equipment has exactly one special; lower rarities have none"), SpecialCount, Base.Rarity >= EItemRarity::EIR_Epic ? 1 : 0);
            }
        }
        FInventorySlot Unrolled;
        Items->CreateInventoryInstance(Unrolled, ID, 20, false);
        TestTrue(TEXT("Random generation flag is honored"), Unrolled.ItemInstance.PrefixOptions.IsEmpty() && Unrolled.ItemInstance.SuffixOptions.IsEmpty());
    }
    FInventorySlot Explicit;
    Items->CreateInventoryInstance(Explicit, TEXT("Weapon_Sword_02"), 1, false);
    TestTrue(TEXT("Family key resolves an eligible tier"), Items->TryAddItemOptionByKey(
        Explicit.ItemInstance, TEXT("Keen"), EItemOptionType::IOT_Prefix, 1));
    TestFalse(TEXT("Explicit duplicate family rejected"), Items->TryAddItemOptionByKey(
        Explicit.ItemInstance, TEXT("Keen_Tier3"), EItemOptionType::IOT_Prefix, 1));
    TestFalse(TEXT("High-level tier rejected at level 1"), Items->TryAddItemOptionByKey(
        Explicit.ItemInstance, TEXT("Precise_Tier1"), EItemOptionType::IOT_Prefix, 1));
    FInventorySlot SpecialItem;
    Items->CreateInventoryInstance(SpecialItem, TEXT("Weapon_Sword_03"), 50, false);
    TestTrue(TEXT("Explicit epic+ special allowed"), Items->TryAddItemOptionByKey(
        SpecialItem.ItemInstance, TEXT("Berserker"), EItemOptionType::IOT_Suffix, 50));
    TestFalse(TEXT("Two different specials on one item are rejected"), Items->TryAddItemOptionByKey(
        SpecialItem.ItemInstance, TEXT("Execution"), EItemOptionType::IOT_Suffix, 50));
    TestFalse(TEXT("Rare items cannot receive specials explicitly"), Items->TryAddItemOptionByKey(
        Explicit.ItemInstance, TEXT("Execution"), EItemOptionType::IOT_Suffix, 1));
    if (!TestEqual(TEXT("Explicit special was stored"), SpecialItem.ItemInstance.SuffixOptions.Num(), 1)) return false;
    const FInventorySlot SpecialRoundTrip = SpecialItem.ToEquippItem().ToInventorySlot(0, 0);
    TestEqual(TEXT("Equipment transfers preserve the special roll"), SpecialRoundTrip.ItemInstance.SuffixOptions[0].Value,
        SpecialItem.ItemInstance.SuffixOptions[0].Value);
    for (const TPair<FName, FName>& Signature : TArray<TPair<FName, FName>>{
        {TEXT("Weapon_Sword_03"), TEXT("Berserker")}, {TEXT("Armor_Shield_03"), TEXT("Lifeline")},
        {TEXT("Acc_Amulet_02"), TEXT("Frostbound")}, {TEXT("Armor_Chest_03"), TEXT("Guardian")}})
    {
        FInventorySlot Generated;
        Items->CreateInventoryInstance(Generated, Signature.Key, 1, true);
        TestTrue(TEXT("Signature special is guaranteed"), Generated.ItemInstance.SuffixOptions.ContainsByPredicate(
            [&](const FDiaActualItemOption& Option) { return Option.OptionID == Signature.Value; }));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaEquipmentRangeTest, "ARPG.Equipment.OptionValueBounds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDiaEquipmentRangeTest::RunTest(const FString& Parameters)
{
    FDiaItemOptionRow Row;
    Row.MinValue = 0.5f;
    Row.MaxValue = 1.5f;
    for (int32 Index = 0; Index < 10000; ++Index)
    {
        const double Value = Row.MakeRandomValue();
        if (!TestTrue(TEXT("Rolled value remains within declared range"), Value >= 0.5 && Value <= 1.5)) return false;
    }
    return true;
}

#endif
