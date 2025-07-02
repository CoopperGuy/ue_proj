// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/ItemName.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"

#include "Controller/DiaController.h"
#include "Character/DiaCharacter.h"

void UItemName::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UItemName::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	OnItemNameClicked.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("Widget clicked!"));
	return FReply::Handled();
}

void UItemName::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	FText StyleNewName = FText::Format(FText::FromString("{0}"), ItemName);

	if (IsValid(ItemNameText))
	{
		ItemNameText->SetText(StyleNewName);
	}
}

void UItemName::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	FText StyleNewName = FText::Format(FText::FromString("<ItemName>{0}</ItemName>"), ItemName);

	if (IsValid(ItemNameText))
	{
		ItemNameText->SetText(StyleNewName);
	}
}

void UItemName::SetItemName(const FText& NewName)
{
	ItemName = NewName;
	FText StyleNewName = FText::Format(FText::FromString("<ItemName>{0}</ItemName>"), ItemName);

	if (IsValid(ItemNameText))
	{
		ItemNameText->SetText(StyleNewName);
	}
}
