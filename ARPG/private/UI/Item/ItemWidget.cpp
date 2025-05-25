// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/ItemWidget.h"
#include "Components/Image.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UItemWidget::SetItemInfo(const FInventoryItem& ItemData)
{
	ItemInfo = ItemData;
	if (ItemIcon)
	{
		ItemIcon->SetColorAndOpacity(FLinearColor::MakeRandomColor());
			//UTexture2D* Icon = ItemInfo.GetIcon();
		//if (Icon)
		//{
		//	ItemIcon->SetBrushFromTexture(Icon);
		//}
	}
}
