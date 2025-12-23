// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/DiaGE_OptionGeneric.h"

#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"

UDiaGE_OptionGeneric::UDiaGE_OptionGeneric()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	
	Modifiers.Empty();
	
	StackingType = EGameplayEffectStackingType::AggregateBySource;
	StackLimitCount = INT_LEAST32_MAX;
	UE_LOG(LogTemp, Warning, TEXT("DiaGE_OptionGeneric::UDiaGE_OptionGeneric - Initializing Modifiers"));
	//아이템 옵션 관련 태그들 적용. 만약 stat이면, attribute에서 가져와야한다.
	for (const FGameplayTag& ItemOptionTag : FDiaGameplayTags::Get().GetItemOptionList())
	{
		FSetByCallerFloat SetByCallerMagnitude;
		SetByCallerMagnitude.DataTag = ItemOptionTag;
        
		FGameplayModifierInfo Modifier;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);

		// 태그 이름에서 연산 타입 결정
		FString TagString = ItemOptionTag.ToString();
		if (TagString.Contains(TEXT("MultiplyAdditive")))
		{
			Modifier.ModifierOp = EGameplayModOp::Multiplicitive;
		}
		else if (TagString.Contains(TEXT("Additive")))
		{
			Modifier.ModifierOp = EGameplayModOp::Additive;
		}
		
		
		FGameplayAttribute TargetAttribute;
		// Attribute가 있는 옵션만 Modifier로 추가
		// LifeSteal 같은 옵션은 Modifier 없이 SetByCaller로만 값 저장 (Exec_Damage에서 읽음)
		if (UDiaAttributeSet::TranslateAttributeTagToAttribute(ItemOptionTag, TargetAttribute))
		{
			Modifier.Attribute = TargetAttribute;
			Modifiers.Add(Modifier);
			UE_LOG(LogTemp, Log, TEXT("DiaGE_OptionGeneric - Modifier 추가: Tag=%s, Attr=%s"),
				*ItemOptionTag.ToString(), *TargetAttribute.GetName());
		}
	}
}
