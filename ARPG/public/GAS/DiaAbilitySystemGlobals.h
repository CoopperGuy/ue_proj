#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "DiaAbilitySystemGlobals.generated.h"

/**
 * Custom AbilitySystemGlobals for Dia project
 * Handles global GAS configuration and initialization
 */
UCLASS()
class ARPG_API UDiaAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	UDiaAbilitySystemGlobals();

	virtual void InitGlobalData() override;
};