#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DiaGameplayEffect.generated.h"

/**
 * Base GameplayEffect class for Dia system
 * Provides common functionality and data structures
 */
UCLASS(BlueprintType, Blueprintable)
class ARPG_API UDiaGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDiaGameplayEffect();
};