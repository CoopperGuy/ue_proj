// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Rewards/RewardChoicePanel.h"
#include "UI/Rewards/RewardCard.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void URewardChoicePanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	RewardCards = { RewardCard_0, RewardCard_1, RewardCard_2 };

	for(URewardCard* RewardCard : RewardCards)
	{
		if(RewardCard)
		{
			RewardCard->OnRewardCardClicked.AddUniqueDynamic(this, &URewardChoicePanel::OnRewardCardClicked);
		}
	}

	DimBackground->SetVisibility(ESlateVisibility::Visible);
}

void URewardChoicePanel::OpenRewardChoicePanel(const FText& Title, const FText& Subtitle, const TArray<FRewardData>& RewardOptions)
{
	if (TitleText)
	{
		TitleText->SetText(Title);
	}

	if (SubtitleText)
	{
		SubtitleText->SetText(Subtitle);
	}

	for(int32 i = 0; i < RewardCards.Num(); ++i)
	{
		if (!RewardCards[i])
		{
			continue;
		}

		if (RewardOptions.IsValidIndex(i))
		{
			RewardCards[i]->SetRewardData(RewardOptions[i]);
			RewardCards[i]->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			RewardCards[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	SetVisibility(ESlateVisibility::Visible);
}

void URewardChoicePanel::CloseRewardChoicePanel()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void URewardChoicePanel::OnRewardCardClicked(URewardCard* ClickedCard)
{
	UE_LOG(LogTemp, Warning, TEXT("Reward Card Clicked: %s"), *ClickedCard->GetName());

	OnRewardCardSelected.Broadcast(ClickedCard->GetRewardData());
	CloseRewardChoicePanel();
}
