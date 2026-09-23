#include "GAS/Executor/DiaStepVariantSupport.h"

#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "DiaBaseCharacter.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/DiaSkillActorSpawnHelper.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"
#include "Skill/DiaSkillActor.h"

namespace DiaStepVariants
{
TArray<UDiaSkillVariant*> Collect(const UDiaGameplayAbility& Ability)
{
	TArray<UDiaSkillVariant*> Result;
	if (const ADiaBaseCharacter* Avatar = Cast<ADiaBaseCharacter>(Ability.GetAvatarActorFromActorInfo()))
	{
		if (UDiaSkillManagerComponent* Skills = Avatar->FindComponentByClass<UDiaSkillManagerComponent>())
		{
			Skills->MakeSkillVariantsArray(&Ability, Result);
		}
	}
	// Applied IDs are a TSet; keep evaluation deterministic across activations.
	Result.RemoveAll([](const UDiaSkillVariant* Variant) { return !IsValid(Variant); });
	Result.Sort([](const UDiaSkillVariant& A, const UDiaSkillVariant& B) { return A.GetSkillID() < B.GetSkillID(); });
	return Result;
}

float Product(const TArray<UDiaSkillVariant*>& Variants, const FGameplayTag& Tag)
{
	float Result = 1.f;
	for (const UDiaSkillVariant* Variant : Variants)
	{
		if (IsValid(Variant) && Variant->GetVariantSpec().SkillTag.MatchesTagExact(Tag))
		{
			Result *= FMath::Max(0.f, Variant->GetVariantSpec().ModifierValue);
		}
	}
	return Result;
}

float Maximum(const TArray<UDiaSkillVariant*>& Variants, const FGameplayTag& Tag)
{
	float Result = 0.f;
	for (const UDiaSkillVariant* Variant : Variants)
	{
		if (IsValid(Variant) && Variant->GetVariantSpec().SkillTag.MatchesTagExact(Tag))
		{
			Result = FMath::Max(Result, Variant->GetVariantSpec().ModifierValue);
		}
	}
	return Result;
}

TArray<FGameplayEffectSpecHandle> MakeEffects(const UDiaGameplayAbility& Ability, const TArray<FSkillVariantTargetEffectData>& Effects)
{
	TArray<FGameplayEffectSpecHandle> Result;
	UAbilitySystemComponent* ASC = Ability.GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC)) return Result;
	for (const FSkillVariantTargetEffectData& Effect : Effects)
	{
		if (!Effect.EffectClass) continue;
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Effect.EffectClass, Ability.GetAbilityLevel(), ASC->MakeEffectContext());
		if (!Spec.IsValid()) continue;
		for (const auto& Pair : Effect.SetByCallerMagnitudes)
		{
			Spec.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
		}
		Result.Add(Spec);
	}
	return Result;
}

void ApplyTargetEffects(const UDiaGameplayAbility& Ability, UAbilitySystemComponent* TargetASC)
{
	UAbilitySystemComponent* ASC = Ability.GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC) || !IsValid(TargetASC) || TargetASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Invincible)) return;
	const FGameplayTag SelfTag = FGameplayTag::RequestGameplayTag(TEXT("GASData.Variant.CastSelfEffect"), false);
	for (const UDiaSkillVariant* Variant : Collect(Ability))
	{
		const FDiaSkillVariantSpec& Spec = Variant->GetVariantSpec();
		if (Spec.SkillTag.MatchesTagExact(SelfTag)) continue;
		if (const FSkillVariantGameplayEffectData* Effects = Spec.GetVariantExtraPtr<FSkillVariantGameplayEffectData>())
		{
			for (const FGameplayEffectSpecHandle& Handle : MakeEffects(Ability, Effects->TargetEffects))
			{
				ASC->ApplyGameplayEffectSpecToTarget(*Handle.Data.Get(), TargetASC);
			}
		}
	}
}

void ApplyCastSelfEffects(const UDiaGameplayAbility& Ability)
{
	UAbilitySystemComponent* ASC = Ability.GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC)) return;
	const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("GASData.Variant.CastSelfEffect"), false);
	for (const UDiaSkillVariant* Variant : Collect(Ability))
	{
		const FDiaSkillVariantSpec& Spec = Variant->GetVariantSpec();
		if (!Spec.SkillTag.MatchesTagExact(Tag)) continue;
		if (const FSkillVariantGameplayEffectData* Effects = Spec.GetVariantExtraPtr<FSkillVariantGameplayEffectData>())
		{
			for (const FGameplayEffectSpecHandle& Handle : MakeEffects(Ability, Effects->TargetEffects))
			{
				ASC->ApplyGameplayEffectSpecToSelf(*Handle.Data.Get());
			}
		}
	}
}

void SpawnCastGrounds(UDiaGameplayAbility& Ability, const TArray<UDiaSkillVariant*>& Variants, const FVector& Location)
{
	AActor* Avatar = Ability.GetAvatarActorFromActorInfo();
	if (!IsValid(Avatar) || !Avatar->HasAuthority()) return;
	const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(TEXT("GASData.Variant.CastGround"), false);
	for (const UDiaSkillVariant* Variant : Variants)
	{
		const FDiaSkillVariantSpec& Spec = Variant->GetVariantSpec();
		if (!Spec.SkillTag.MatchesTagExact(Tag)) continue;
		const FSkillVariantGroundSpawnData* Ground = Spec.GetVariantExtraPtr<FSkillVariantGroundSpawnData>();
		if (!Ground || !Ground->SkillActorClass || Ground->Duration <= 0.f || Ground->TickInterval <= 0.f) continue;
		// CastGround's magnitude is the field's base damage; it must not inherit teleport range or healing power.
		FGASSkillData FieldData = Ability.GetSkillData();
		FieldData.ExtraData.Reset();
		FieldData.BaseDamage = FMath::Max(0.f, Spec.ModifierValue);
		FieldData.HitInterval = Ground->TickInterval;
		FieldData.HitCount = FMath::Max(1, FMath::CeilToInt(Ground->Duration / Ground->TickInterval));
		FDiaSkillActorSpawnRequest Request;
		Request.World = Avatar->GetWorld();
		Request.SkillActorClass = Ground->SkillActorClass;
		FVector GroundLocation = Location;
		if (const ACharacter* CharacterAvatar = Cast<ACharacter>(Avatar))
		{
			GroundLocation.Z -= CharacterAvatar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}
		Request.SpawnTransform = FTransform(FRotator::ZeroRotator, GroundLocation);
		Request.OwnerActor = Avatar;
		Request.Instigator = Cast<APawn>(Avatar);
		Request.SkillData = &FieldData;
		Request.SourceASC = Ability.GetAbilitySystemComponentFromActorInfo();
		Request.DamageEffectClass = Ground->DamageEffectClass ? Ground->DamageEffectClass : Ability.GetDamageEffectClass();
		Request.OwningAbility = &Ability;
		Request.TargetEffectSpecs = MakeEffects(Ability, Ground->TargetEffects);
		Request.bSpawnedByFork = true;
		Request.bLaunch = false;
		Request.bApplyAbilityRemovalTimer = false;
		if (ADiaSkillActor* Actor = FDiaSkillActorSpawnHelper::SpawnSkillActor(Request))
		{
			FDiaSkillActorConfigureData Config;
			Config.AreaRadius = Ground->Radius;
			Config.Duration = Ground->Duration;
			Config.HitInterval = Ground->TickInterval;
			Actor->ConfigureSkillActor(Config);
			Actor->ArmRemovalTimer(Ground->Duration);
		}
	}
}
}
