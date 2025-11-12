// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "ItemName.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemNameClicked);
class UTextBlock;
class URichTextBlock;
/**
 * 
 */
UCLASS()
class ARPG_API UItemName : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	void SetItemName(const FText& NewName);
public:
	UPROPERTY()
	FOnItemNameClicked OnItemNameClicked;
protected:
	UPROPERTY(meta = (BindWidget))
	URichTextBlock* ItemNameText;

	FText ItemName;


};
