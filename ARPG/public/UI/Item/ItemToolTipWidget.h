// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "ItemToolTipWidget.generated.h"

class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class ARPG_API UItemToolTipWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetToolTipItem(const FInventorySlot& InItem);
	
protected:
	FInventorySlot ToolTipItem;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemName;
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemDesc;
};
