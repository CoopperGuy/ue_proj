// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/DiaGE_StatApply.h"
#include "GameplayEffectTypes.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"


//스탯 적용이기 때문에 무한하게 적용한다.
UDiaGE_StatApply::UDiaGE_StatApply()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;

	//AttributeSet에 정의된 모든 스탯에 대하여 Modifiyer를 추가한다.
	//FDiaGameplayTags::Get().GetAttributeStats()를 사용하여 모든 스탯 태그를 가져온다.
	//해당 AttributeSet에 모두 다 정의해놔야한다.
	
	// Modifiers 배열 초기화 

	//PostLoad()에 옮기는 방법도 생각해야한다

	//초기화 문제로 안됐었음... 
	Modifiers.Empty();
	
#ifdef WITH_EDITOR
	int32 AddedCount = 0;
#endif // WITH_EDITOR

	for (const FGameplayTag& AttributeTag : FDiaGameplayTags::Get().GetAttributeStats())
	{
		FSetByCallerFloat SetByCallerMagnitude;
		SetByCallerMagnitude.DataTag = AttributeTag;
		
		FGameplayAttribute DummyAttribute;
		const bool bTranslated = UDiaAttributeSet::TranslateAttributeTagToAttribute(AttributeTag, DummyAttribute);
		if (bTranslated)
		{
			FGameplayModifierInfo Modifier;
			Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);
			Modifier.ModifierOp = EGameplayModOp::Additive;
			Modifier.Attribute = DummyAttribute;
			Modifiers.Add(Modifier);
#ifdef WITH_EDITOR
			AddedCount++;
#endif // WITH_EDITOR

		}
		
		UE_LOG(LogTemp, Warning,
			TEXT("StatApply Tag: %s, Translate: %s"),
			*AttributeTag.ToString(),
			bTranslated ? TEXT("OK") : TEXT("FAIL"));
	
		
	}
#ifdef WITH_EDITOR
	UE_LOG(LogTemp, Warning, TEXT("UDiaGE_StatApply::Constructor - Added %d Modifiers, Total: %d"), AddedCount, Modifiers.Num());
#endif // WITH_EDITOR

}
