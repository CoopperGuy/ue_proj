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
		FText DescriptionText;
		for (const auto& option : ToolTipItem.ItemInstance.PrefixOptions)
		{
			DescriptionText = DescriptionText.FromString(DescriptionText.ToString() + option.GetOptionDescription().ToString() + TEXT("\n"));
		}
		for (const auto& option : ToolTipItem.ItemInstance.SuffixOptions)
		{
			DescriptionText = DescriptionText.FromString(DescriptionText.ToString() + option.GetOptionDescription().ToString() + TEXT("\n"));
		}

		ItemDesc->SetText(DescriptionText);
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
