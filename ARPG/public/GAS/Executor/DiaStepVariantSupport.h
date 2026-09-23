#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"

class UDiaGameplayAbility;
class UDiaSkillVariant;
class UAbilitySystemComponent;
struct FSkillVariantTargetEffectData;

/** Step 실행에서 사용하는 variant 처리. 레거시 전용 Ability를 새로 만들지 않는다. */
namespace DiaStepVariants
{
	ARPG_API TArray<UDiaSkillVariant*> Collect(const UDiaGameplayAbility& Ability);
	ARPG_API float Product(const TArray<UDiaSkillVariant*>& Variants, const FGameplayTag& Tag);
	ARPG_API float Maximum(const TArray<UDiaSkillVariant*>& Variants, const FGameplayTag& Tag);
	ARPG_API TArray<FGameplayEffectSpecHandle> MakeEffects(const UDiaGameplayAbility& Ability, const TArray<FSkillVariantTargetEffectData>& Effects);
	ARPG_API void ApplyTargetEffects(const UDiaGameplayAbility& Ability, UAbilitySystemComponent* TargetASC);
	ARPG_API void ApplyCastSelfEffects(const UDiaGameplayAbility& Ability);
	ARPG_API void SpawnCastGrounds(UDiaGameplayAbility& Ability, const TArray<UDiaSkillVariant*>& Variants, const FVector& Location);
}
