// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Types/DiaRewardData.h"
#include "RewardCard.generated.h"

class UButton;
class UCanvasPanel;
class UImage;
class UTextBlock;
class URewardCard;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardCardClicked, URewardCard*, RewardCard);

/**
 * 
 */
UCLASS()
class ARPG_API URewardCard : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	void SetRewardData(const FRewardData& InRewardData);
	
	UFUNCTION()
	void OnCardButtonClicked();

public:
	FOnRewardCardClicked OnRewardCardClicked;
protected:
	UPROPERTY(meta = (BindWidget))
	UButton* CardButton;
	UPROPERTY(meta = (BindWidget))
	UImage* CardBackground;

	UPROPERTY(meta = (BindWidget))
	UImage* RarityStripe;

	UPROPERTY(meta = (BindWidget))
	UImage* IconFrame;

	UPROPERTY(meta = (BindWidget))
	UImage* RewardIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RewardTypeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RewardRarityText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RewardNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RewardDescriptionText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RewardEffectText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectHintText;
};
