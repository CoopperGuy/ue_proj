// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/ItemToolTipWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"


void UItemToolTipWidget::SetToolTipItem(const FInventorySlot& InItem)
{
	ToolTipItem = InItem;
	if (ItemName)
	{
		ItemName->SetText(ToolTipItem.ItemInstance.GetDisplayName());
	}
	if (ItemDesc)
	{
		ItemDesc->SetText(ToolTipItem.ItemInstance.GetDescription());
	}
	if (ItemIcon)
	{
		if (ToolTipItem.ItemInstance.GetIconPath().IsValid())
		{
			// FSoftObjectPath를 통해 텍스처 비동기 로딩
			TSoftObjectPtr<UTexture2D> IconTexture(ToolTipItem.ItemInstance.GetIconPath());
			if (UTexture2D* Icon = IconTexture.LoadSynchronous())
			{
				ItemIcon->SetBrushFromTexture(Icon);
			}
		}
	}
}
