#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DiaGameplayEffect_Damage.generated.h"

/**
 * 대미지 적용을 위한 GameplayEffect
 */
UCLASS()
class ARPG_API UDiaGameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDiaGameplayEffect_Damage();
};