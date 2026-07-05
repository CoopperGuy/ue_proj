// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Rewards/RewardCard.h"

#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


void URewardCard::SetRewardData(const FRewardData& InRewardData)
{
	if (RewardTypeText)
	{
		const UEnum* RewardTypeEnumPtr = StaticEnum<ERewardType>();
		FString GradeString = RewardTypeEnumPtr->GetNameStringByValue(static_cast<int64>(InRewardData.RewardType));

		RewardTypeText->SetText(FText::FromString(GradeString));
	}
	
	if (RewardRarityText)
	{
		const UEnum* RewardRarityEnumPtr = StaticEnum<ERewardRarity>();
		FString RareString = RewardRarityEnumPtr->GetNameStringByValue(static_cast<int64>(InRewardData.RewardRarity));

		RewardRarityText->SetText(FText::FromString(RareString));
	}

	if(RewardNameText)
	{
		RewardNameText->SetText(InRewardData.DisplayName);
	}

	if(RewardDescriptionText)
	{
		RewardDescriptionText->SetText(InRewardData.Description);
	}

	if (RewardEffectText)
	{
		FString EffectText;
		switch (InRewardData.RewardType)
		{
		case ERewardType::Gold:
			EffectText = FString::Printf(TEXT("+%d"), InRewardData.GoldAmount);
			break;
		case ERewardType::Item:
			EffectText = FString::Printf(TEXT("Item ID: %s"), *InRewardData.ItemId.ToString());
			break;
		case ERewardType::SkillAdd:
			EffectText = FString::Printf(TEXT("Skill ID: %d"), InRewardData.SkillId);
			break;
		case ERewardType::SkillUpgrade:
			EffectText = FString::Printf(TEXT("Skill ID: %d"), InRewardData.SkillId);
			break;
		case ERewardType::SkillVariant:
			EffectText = FString::Printf(TEXT("Skill ID: %d, Variant ID: %d"), InRewardData.SkillId, InRewardData.VariantId);
			break;
		default:
			EffectText = TEXT("");
			break;
		}
		RewardEffectText->SetText(FText::FromString(EffectText));
	}
}

void URewardCard::OnCardButtonClicked()
{
	OnRewardCardClicked.Broadcast(this);
}
