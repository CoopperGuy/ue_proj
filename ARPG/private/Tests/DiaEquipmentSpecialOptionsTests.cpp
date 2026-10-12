#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"
#include "GAS/Effects/DiaGE_OptionGeneric.h"
#include "Types/DiaEquipmentOptionRules.h"
#include "Types/DiaItemOptionRow.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaEquipmentProcRulesTest, "ARPG.Equipment.Special.ProcBoundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDiaEquipmentProcRulesTest::RunTest(const FString& Parameters)
{
    using namespace DiaEquipmentOptionRules;
    FProcCooldowns Cooldowns;
    const FGameplayTag Frost = DiaGameplayTags::ItemOption_Frostbound;
    const FGameplayTag Lifeline = DiaGameplayTags::ItemOption_Lifeline;
    TestFalse(TEXT("Failed proc does not consume cooldown"), Cooldowns.TryActivate(Frost, 0., FrostChance, .9f, FrostCooldown));
    TestTrue(TEXT("Successful first hit is ready even at time zero"), Cooldowns.TryActivate(Frost, 0., FrostChance, .1f, FrostCooldown));
    TestFalse(TEXT("Multihits share the cooldown"), Cooldowns.TryActivate(Frost, .1, FrostChance, .1f, FrostCooldown));
    TestTrue(TEXT("Other families have independent cooldowns"), Cooldowns.TryActivate(Lifeline, .1, 1.f, .9f, LifelineCooldown));
    TestFalse(TEXT("Still cooling down immediately before the boundary"), Cooldowns.TryActivate(Frost, 2.999, FrostChance, .1f, FrostCooldown));
    TestTrue(TEXT("Ready at the exact boundary"), Cooldowns.TryActivate(Frost, 3., FrostChance, .1f, FrostCooldown));
    TestTrue(TEXT("35 percent is low health"), IsLowHealth(35.f, 100.f));
    TestFalse(TEXT("Above threshold is not low health"), IsLowHealth(35.01f, 100.f));
    TestFalse(TEXT("Lifeline cannot resurrect"), IsLowHealth(0.f, 100.f));
    TestEqual(TEXT("Overkill only counts remaining health"), ConfirmedHealthLoss(10.f, -90.f), 10.f);
    TestEqual(TEXT("No damage gives no recovery"), ConfirmedHealthLoss(100.f, 100.f), 0.f);
    TestEqual(TEXT("Healing cannot trigger damage procs"), ConfirmedHealthLoss(50.f, 80.f), 0.f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaEquipmentOptionMappingTest, "ARPG.Equipment.Special.AttributeMappings",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDiaEquipmentOptionMappingTest::RunTest(const FString& Parameters)
{
    using namespace DiaGameplayTags;
    const TMap<FGameplayTag, FGameplayAttribute> Expected = {
        {ItemOption_AttackPower, UDiaAttributeSet::GetAttackPowerAttribute()},
        {ItemOption_Defense, UDiaAttributeSet::GetDefenseAttribute()},
        {ItemOption_MaxMana, UDiaAttributeSet::GetMaxManaAttribute()},
        {ItemOption_CritDamage, UDiaAttributeSet::GetCritDamageAttribute()}
    };
    const UDiaGE_OptionGeneric* Effect = GetDefault<UDiaGE_OptionGeneric>();
    for (const auto& Pair : Expected)
    {
        FGameplayAttribute Attribute;
        TestTrue(TEXT("New affix is accepted by the generator"), FDiaGameplayTags::GetItemOptionList().Contains(Pair.Key));
        TestTrue(TEXT("Affix maps to a combat attribute"), UDiaAttributeSet::TranslateAttributeTagToAttribute(Pair.Key, Attribute));
        TestTrue(TEXT("Correct target attribute"), Attribute == Pair.Value);
        TestTrue(TEXT("Equipment effect modifies the attribute"), Effect->Modifiers.ContainsByPredicate([&](const FGameplayModifierInfo& Modifier)
        {
            return Modifier.Attribute == Pair.Value && Modifier.ModifierOp == EGameplayModOp::Additive;
        }));
    }
    for (const FGameplayTag& Tag : {ItemOption_Execution.GetTag(), ItemOption_Ambush.GetTag(), ItemOption_Berserker.GetTag(),
        ItemOption_Guardian.GetTag(), ItemOption_Frostbound.GetTag(), ItemOption_Lifeline.GetTag()})
    {
        FGameplayAttribute Attribute;
        TestFalse(TEXT("Special options never become unconditional stat modifiers"), UDiaAttributeSet::TranslateAttributeTagToAttribute(Tag, Attribute));
        FDiaActualItemOption Option;
        Option.GrantedTag = Tag;
        Option.Value = 20.f;
        const FString Description = Option.GetOptionDescription().ToString();
        TestFalse(TEXT("Every special family has a readable description"), Description.IsEmpty());
        TestFalse(TEXT("Internal gameplay tags stay out of tooltip text"), Description.Contains(TEXT("Item.Option")));
    }
    TestEqual(TEXT("Critical damage bonuses add to a usable base multiplier"), GetDefault<UDiaAttributeSet>()->GetCritDamage(), 150.f);
    return true;
}

#endif
