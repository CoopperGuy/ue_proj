#pragma once

#include "GameplayEffect.h"
#include "UObject/UnrealType.h"

namespace DiaGameplayEffectCompat
{
	inline void SetStackingType(UGameplayEffect* GameplayEffect, EGameplayEffectStackingType StackingType)
	{
		if (!GameplayEffect)
		{
			return;
		}

		FProperty* Property = FindFProperty<FProperty>(UGameplayEffect::StaticClass(), TEXT("StackingType"));
		if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
		{
			ByteProperty->SetPropertyValue_InContainer(GameplayEffect, static_cast<uint8>(StackingType));
			return;
		}

		if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(
				EnumProperty->ContainerPtrToValuePtr<void>(GameplayEffect),
				static_cast<uint64>(StackingType));
		}
	}
}
