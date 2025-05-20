// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/ItemName.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"

void UItemName::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UItemName::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("Widget clicked!"));
	return FReply::Handled();
}

void UItemName::SetItemName(const FText& NewName)
{
	FText StyleNewName = FText::Format(FText::FromString("<ItemName>{0}</ItemName>"), NewName);

	if (IsValid(ItemNameText))
	{
		ItemNameText->SetText(StyleNewName);
	}

}
