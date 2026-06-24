// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/ItemToolTipWidget.h"
#include "System/ItemSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

namespace
{
	FString GetStatDisplayName(const FGameplayTag& StatTag)
	{
		const FString TagString = StatTag.ToString();

		if (TagString == TEXT("AttributeSet.Defense"))
		{
			return TEXT("방어력");
		}
		if (TagString == TEXT("AttributeSet.MaxHealth"))
		{
			return TEXT("최대 체력");
		}
		if (TagString == TEXT("AttributeSet.AttackPower"))
		{
			return TEXT("공격력");
		}
		if (TagString == TEXT("AttributeSet.Strength"))
		{
			return TEXT("힘");
		}
		if (TagString == TEXT("AttributeSet.Dexterity"))
		{
			return TEXT("민첩");
		}
		if (TagString == TEXT("AttributeSet.MovementSpeed"))
		{
			return TEXT("이동 속도");
		}

		FString Left;
		FString Right;
		return TagString.Split(TEXT("."), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd)
			? Right
			: TagString;
	}
}

void UItemToolTipWidget::SetToolTipItem(const FInventorySlot& InItem)
{
	ToolTipItem = InItem;
	UItemSubsystem* ItemSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
	if (ItemName)
	{
		ItemName->SetText(ItemSubsystem ? ItemSubsystem->GetItemDisplayName(ToolTipItem.ItemInstance) : FText::FromName(ToolTipItem.ItemInstance.ItemID));
	}
	if (ItemDesc)
	{
		FString DescriptionString;

		if (!ToolTipItem.ItemInstance.ModifiedStats.IsEmpty())
		{
			DescriptionString += TEXT("기본 능력치\n");
			for (const auto& StatPair : ToolTipItem.ItemInstance.ModifiedStats)
			{
				DescriptionString += FString::Printf(TEXT("%s +%.0f\n"),
					*GetStatDisplayName(StatPair.Key),
					StatPair.Value.Values);
			}
		}

		const bool bHasOptions = ToolTipItem.ItemInstance.PrefixOptions.Num() > 0
			|| ToolTipItem.ItemInstance.SuffixOptions.Num() > 0;
		if (bHasOptions)
		{
			if (!DescriptionString.IsEmpty())
			{
				DescriptionString += TEXT("\n");
			}
			DescriptionString += TEXT("옵션\n");
		}

		for (const auto& option : ToolTipItem.ItemInstance.PrefixOptions)
		{
			DescriptionString += option.GetOptionDescription().ToString();
			DescriptionString += TEXT("\n");
		}
		for (const auto& option : ToolTipItem.ItemInstance.SuffixOptions)
		{
			DescriptionString += option.GetOptionDescription().ToString();
			DescriptionString += TEXT("\n");
		}

		ItemDesc->SetText(FText::FromString(DescriptionString));
	}
	if (ItemIcon)
	{
		const FSoftObjectPath IconPath = ItemSubsystem ? ItemSubsystem->GetIconPath(ToolTipItem.ItemInstance) : FSoftObjectPath();
		if (IconPath.IsValid())
		{
			// FSoftObjectPath를 통해 텍스처 비동기 로딩
			TSoftObjectPtr<UTexture2D> IconTexture(IconPath);
			if (UTexture2D* Icon = IconTexture.LoadSynchronous())
			{
				ItemIcon->SetBrushFromTexture(Icon);
			}
		}
	}
}
