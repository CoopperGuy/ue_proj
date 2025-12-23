#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"

#include "Skill/DiaSkillType.h"
#include "Types/DiaGASSkillData.h"

#include "DiaGASHelper.generated.h"

class UAbilitySystemComponent;
class UDiaGameplayAbility;

/**
 * Helper functions for GAS integration with existing skill system
 */
UCLASS()
class ARPG_API UDiaGASHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Convert skill data to gameplay ability
	UFUNCTION(BlueprintCallable, Category = "GAS|Helper")
	static bool GrantAbilityFromSkillData(UAbilitySystemComponent* ASC, const FGASSkillData& SkillData, int32 SkillID, FGameplayTag AbilityTag = FGameplayTag());

	// Remove ability by skill ID
	UFUNCTION(BlueprintCallable, Category = "GAS|Helper")
	static bool RevokeAbilityBySkillID(UAbilitySystemComponent* ASC, int32 SkillID);

	// Try activate ability by skill ID
	UFUNCTION(BlueprintCallable, Category = "GAS|Helper")
	static bool TryActivateAbilityBySkillID(UAbilitySystemComponent* ASC, int32 SkillID);

	// Check if ability can be activated by skill ID
	UFUNCTION(BlueprintCallable, Category = "GAS|Helper")
	static bool CanActivateAbilityBySkillID(UAbilitySystemComponent* ASC, int32 SkillID);

	// Get ability spec by skill ID (C++ only - not exposed to Blueprint)
	static FGameplayAbilitySpec* GetAbilitySpecBySkillID(UAbilitySystemComponent* ASC, int32 SkillID);

	// Get cooldown remaining for skill ID
	UFUNCTION(BlueprintCallable, Category = "GAS|Helper")
	static float GetCooldownRemainingBySkillID(UAbilitySystemComponent* ASC, int32 SkillID);

	static float GetCooldownRatioBySkillID(UAbilitySystemComponent* ASC, int32 SkillID);

	static FVector GetMouseWorldLocation(const FGameplayAbilityActorInfo& ActorInfo);
private:
	// Get ability class from skill data
	static TSubclassOf<UDiaGameplayAbility> GetAbilityClassFromSkillData(const FGASSkillData& SkillData);
};