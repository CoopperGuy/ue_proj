// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Rewards/RewardChoicePanel.h"
#include "InputCoreTypes.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Rewards/RewardCard.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void URewardChoicePanel::NativeConstruct()
{
	Super::NativeConstruct();
    SetIsFocusable(true);
    SetVisibility(ESlateVisibility::Collapsed);
	
	RewardCards = { RewardCard_0, RewardCard_1, RewardCard_2 };

	for(URewardCard* RewardCard : RewardCards)
	{
		if(RewardCard)
		{
			RewardCard->OnRewardCardClicked.AddUniqueDynamic(this, &URewardChoicePanel::OnRewardCardClicked);
		}
	}

	if (DimBackground) DimBackground->SetVisibility(ESlateVisibility::Visible);
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

    if (ConfirmHintText) ConfirmHintText->SetText(NSLOCTEXT("RoomReward", "Choose", "하나를 선택하세요. Esc: 닫기"));
	SetVisibility(ESlateVisibility::Visible);
    SetFocus();
}

void URewardChoicePanel::CloseRewardChoicePanel()
{
	SetVisibility(ESlateVisibility::Collapsed);
    UWidgetBlueprintLibrary::SetFocusToGameViewport();
}

void URewardChoicePanel::OnRewardCardClicked(URewardCard* ClickedCard)
{
    if (!IsValid(ClickedCard) || !IsVisible()) return;

	OnRewardCardSelected.Broadcast(ClickedCard->GetRewardData());
}

void URewardChoicePanel::ShowClaimFailure()
{
    if (ConfirmHintText) ConfirmHintText->SetText(NSLOCTEXT("RoomReward", "ClaimFailed", "획득할 수 없습니다. 보상대 가까이에서 인벤토리 공간과 스킬 상태를 확인하세요."));
}

FReply URewardChoicePanel::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    if (Event.GetKey() == EKeys::Escape)
    {
        CloseRewardChoicePanel();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(Geometry, Event);
}
