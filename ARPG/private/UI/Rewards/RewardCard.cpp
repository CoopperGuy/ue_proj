// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Rewards/RewardCard.h"

#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


void URewardCard::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (CardButton)
	{
		CardButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnCardButtonClicked);
	}
}

void URewardCard::SetRewardData(const FRewardData& InRewardData)
{
	RewardData = InRewardData;

	if (RewardIcon)
	{
		RewardIcon->SetBrushFromTexture(InRewardData.Icon);
		RewardIcon->SetVisibility(InRewardData.Icon ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (RewardTypeText)
	{
		const UEnum* RewardTypeEnumPtr = StaticEnum<ERewardType>();
		FString GradeString = RewardTypeEnumPtr->GetDisplayNameTextByValue(static_cast<int64>(InRewardData.RewardType)).ToString();

		RewardTypeText->SetText(FText::FromString(GradeString));
	}
	
	if (RewardRarityText)
	{
		const UEnum* RewardRarityEnumPtr = StaticEnum<ERewardRarity>();
		FString RareString = RewardRarityEnumPtr->GetDisplayNameTextByValue(static_cast<int64>(InRewardData.RewardRarity)).ToString();

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
        case ERewardType::Heal:
            EffectText = TEXT("최대 체력의 25% 회복");
            break;
		case ERewardType::Gold:
			EffectText = FString::Printf(TEXT("+%d"), InRewardData.GoldAmount);
			break;
		case ERewardType::Item:
			EffectText = FString::Printf(TEXT("장비 %d개 · 레벨 %d"), InRewardData.ItemQuantity, InRewardData.ItemLevel);
			break;
		case ERewardType::SkillAdd:
			EffectText = TEXT("새 스킬 습득 · 빈 슬롯에 자동 등록");
			break;
		case ERewardType::SkillUpgrade:
			EffectText = TEXT("보유 스킬 레벨 +1");
			break;
		case ERewardType::SkillVariant:
			EffectText = TEXT("스킬 강화 해금 · 스킬 창에서 적용");
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
