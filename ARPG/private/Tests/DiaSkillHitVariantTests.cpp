#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect_Hit.h"
#include "GAS/DiaGameplayTags.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaVariantPreservesHitBudgetTest,
    "ARPG.Skills.Variants.PreservesHitBudget",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDiaVariantPreservesHitBudgetTest::RunTest(const FString& Parameters)
{
    UDiaSkillVariantEffect_Pierce* Effect = NewObject<UDiaSkillVariantEffect_Pierce>();
    FDiaSkillVariantContext Context;
    FSkillHitRuntime Runtime;
    Runtime.PierceCount = 2; // One hit has already been consumed by the actor.

    const FDiaGameplayTags& Tags = FDiaGameplayTags::Get();
    for (const FGameplayTag& Tag : {Tags.GASData_Pierce, Tags.GASData_Variant_Fork, Tags.GASData_Variant_FreezeStack})
    {
        FDiaSkillVariantSpec Spec;
        Spec.SkillTag = Tag;
        Spec.ModifierValue = 3.f;
        Effect->Apply(Spec, Context, Runtime);
    }
    TestEqual(TEXT("Combining variants must not consume extra hits"), Runtime.PierceCount, 2);

    Runtime.PierceCount = 0;
    FDiaSkillVariantSpec Pierce;
    Pierce.SkillTag = Tags.GASData_Pierce;
    Pierce.ModifierValue = 3.f;
    Effect->Apply(Pierce, Context, Runtime);
    TestEqual(TEXT("An exhausted projectile must not regain its hit budget"), Runtime.PierceCount, 0);
    return true;
}

#endif
