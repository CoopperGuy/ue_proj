#include "GAS/DiaEquipmentCombat.h"
#include "AbilitySystemComponent.h"
#include "Controller/DiaController.h"
#include "DiaBaseCharacter.h"
#include "DiaComponent/DiaOptionManagerComponent.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"
#include "GAS/Effects/DiaGE_EquipmentSlow.h"
#include "GAS/Effects/DiaGE_OptionGeneric.h"
#include "Types/DiaEquipmentOptionRules.h"

namespace
{
    float Health(UAbilitySystemComponent* ASC)
    {
        return ASC ? ASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute()) : 0.f;
    }

    float MaxHealth(UAbilitySystemComponent* ASC)
    {
        return ASC ? ASC->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute()) : 0.f;
    }

    bool Proc(UAbilitySystemComponent* ASC, const FGameplayTag& Tag, float Chance, float Cooldown)
    {
        const APawn* Pawn = ASC ? Cast<APawn>(ASC->GetAvatarActor()) : nullptr;
        ADiaController* Controller = Pawn ? Cast<ADiaController>(Pawn->GetController()) : nullptr;
        UDiaOptionManagerComponent* Options = Controller ? Controller->GetOptionManagerComponent() : nullptr;
        return Options && Options->TryActivateEquipmentProc(ASC, Tag, Chance, Cooldown);
    }

    void Restore(UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute,
        const FGameplayAttribute& Maximum, float Amount)
    {
        if (!ASC || Health(ASC) <= 0.f || !FMath::IsFinite(Amount) || Amount <= 0.f) return;
        const float Missing = FMath::Max(0.f, ASC->GetNumericAttribute(Maximum) - ASC->GetNumericAttribute(Attribute));
        if (Missing > 0.f) ASC->ApplyModToAttribute(Attribute, EGameplayModOp::Additive, FMath::Min(Missing, Amount));
    }
}

float DiaEquipmentCombat::Magnitude(UAbilitySystemComponent* ASC, const FGameplayTag& Tag)
{
    if (!ASC || !Tag.IsValid()) return 0.f;
    FGameplayEffectQuery Query;
    Query.EffectDefinition = UDiaGE_OptionGeneric::StaticClass();
    float Result = 0.f;
    const bool bStrongest = DiaEquipmentOptionRules::IsSpecial(Tag);
    for (FActiveGameplayEffectHandle Handle : ASC->GetActiveEffects(Query))
    {
        const FActiveGameplayEffect* Effect = ASC->GetActiveGameplayEffect(Handle);
        if (!Effect || Effect->bIsInhibited) continue;
        const float Value = Effect->Spec.GetSetByCallerMagnitude(Tag, false, 0.f);
        if (!FMath::IsFinite(Value) || Value <= 0.f) continue;
        Result = bStrongest ? FMath::Max(Result, Value) : Result + Value;
    }
    return Result;
}

float DiaEquipmentCombat::DamageMultiplier(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target)
{
    using namespace DiaGameplayTags;
    using namespace DiaEquipmentOptionRules;
    float Bonus = 0.f;
    if (IsLowHealth(Health(Target), MaxHealth(Target))) Bonus += Magnitude(Source, ItemOption_Execution);
    if (Health(Target) > 0.f && MaxHealth(Target) > 0.f && Health(Target) >= MaxHealth(Target) * HealthyRatio)
        Bonus += Magnitude(Source, ItemOption_Ambush);
    if (IsLowHealth(Health(Source), MaxHealth(Source))) Bonus += Magnitude(Source, ItemOption_Berserker);
    const float Reduction = IsLowHealth(Health(Target), MaxHealth(Target)) ? Magnitude(Target, ItemOption_Guardian) : 0.f;
    return (1.f + FMath::Clamp(Bonus, 0.f, 100.f) * .01f) * (1.f - FMath::Clamp(Reduction, 0.f, 50.f) * .01f);
}

void DiaEquipmentCombat::OnConfirmedHit(UAbilitySystemComponent* Source, UAbilitySystemComponent* Target,
    float ActualDamage, bool bKilled)
{
    using namespace DiaGameplayTags;
    using namespace DiaEquipmentOptionRules;
    if (!Source || !Target || Source == Target || ActualDamage <= 0.f) return;
    AActor* Victim = Target->GetAvatarActor();
    if (!Victim || !Victim->HasAuthority()) return;

    Restore(Source, UDiaAttributeSet::GetHealthAttribute(), UDiaAttributeSet::GetMaxHealthAttribute(),
        ActualDamage * FMath::Clamp(Magnitude(Source, ItemOption_LifeSteal), 0.f, 50.f) * .01f);
    if (bKilled)
    {
        Restore(Source, UDiaAttributeSet::GetHealthAttribute(), UDiaAttributeSet::GetMaxHealthAttribute(), Magnitude(Source, ItemOption_HealthOnKill));
        Restore(Source, UDiaAttributeSet::GetManaAttribute(), UDiaAttributeSet::GetMaxManaAttribute(), Magnitude(Source, ItemOption_ManaOnKill));
        return;
    }

    const float Lifeline = Magnitude(Target, ItemOption_Lifeline);
    if (Lifeline > 0.f && IsLowHealth(Health(Target), MaxHealth(Target))
        && Proc(Target, ItemOption_Lifeline, 1.f, LifelineCooldown))
    {
        Restore(Target, UDiaAttributeSet::GetHealthAttribute(), UDiaAttributeSet::GetMaxHealthAttribute(),
            MaxHealth(Target) * FMath::Clamp(Lifeline, 0.f, 30.f) * .01f);
    }

    const float Slow = Magnitude(Source, ItemOption_Frostbound);
    if (Slow > 0.f && Health(Source) > 0.f && Health(Target) > 0.f
        && Proc(Source, ItemOption_Frostbound, FrostChance, FrostCooldown))
    {
        FGameplayEffectSpecHandle Spec = Source->MakeOutgoingSpec(UDiaGE_EquipmentSlow::StaticClass(), 1.f, Source->MakeEffectContext());
        if (Spec.IsValid())
        {
            Spec.Data->SetSetByCallerMagnitude(FDiaGameplayTags::Get().GASData_StatusValue, 1.f - FMath::Clamp(Slow, 0.f, 50.f) * .01f);
            Source->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), Target);
        }
    }
}
