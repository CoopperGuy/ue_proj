// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Types/DiaRewardData.h"
#include "RewardChoicePanel.generated.h"

class UCanvasPanel;
class UHorizontalBox;
class UImage;
class URewardCard;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardCardSelect, FRewardData, RewardData);

/**
 * 
 */
UCLASS()
class ARPG_API URewardChoicePanel : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void OpenRewardChoicePanel(const FText& Title, const FText& Subtitle, const TArray<FRewardData>& RewardOptions);
	void CloseRewardChoicePanel();

	UFUNCTION()
	void OnRewardCardClicked(URewardCard* ClickedCard);

public:
	FOnRewardCardSelect OnRewardCardSelected;
protected:
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* RewardChoiceRoot;

	UPROPERTY(meta = (BindWidget))
	UImage* DimBackground;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MainPanel;

	UPROPERTY(meta = (BindWidget))
	UImage* PanelSurface;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SubtitleText;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* RewardCardsBox;

	UPROPERTY(meta = (BindWidget))
	URewardCard* RewardCard_0;

	UPROPERTY(meta = (BindWidget))
	URewardCard* RewardCard_1;

	UPROPERTY(meta = (BindWidget))
	URewardCard* RewardCard_2;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ConfirmHintText;

	UPROPERTY()
	TArray<URewardCard*> RewardCards;
};
