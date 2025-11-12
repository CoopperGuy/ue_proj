// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ItemSubsystem.h"
#include "UI/Item/ItemWidget.h"
#include "UI/Item/ItemToolTipWidget.h"
#include "Types/ItemBase.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
	LoadItemData();
    UE_LOG(LogTemp, Warning, TEXT("ItemSubsystem: LoadComplete! "));

}

void UItemSubsystem::Deinitialize()
{
    Super::Deinitialize();

	ItemDataTable = nullptr;
	ItemCache.Empty();
}

void UItemSubsystem::LoadItemData()
{
    ItemDataTable = LoadObject<UDataTable>(nullptr, *ItemDataTablePath);
    if (ItemDataTable)
    {
        ItemCache.Empty();
        TArray<FName> RowNames = ItemDataTable->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            FItemBase* ItemRow = ItemDataTable->FindRow<FItemBase>(RowName, TEXT(""));
            if (ItemRow)
            {
                ItemCache.Emplace(ItemRow->ItemID, *ItemRow);   
            }
        }
	}
#ifdef UE_BUILD_DEBUG
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemSubsystem: Failed to load item data table from path: %s"), *ItemDataTablePath);
	}
#endif
}

void UItemSubsystem::LoadOptionData()
{
    OptionDataTable = LoadObject<UDataTable>(nullptr, *OptionDataTablePath);
    if (OptionDataTable)
    {
        OptionCache.Empty();
        TArray<FName> RowNames = OptionDataTable->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            FDiaItemOptionRow* OptionRow = OptionDataTable->FindRow<FDiaItemOptionRow>(RowName, TEXT(""));
            if (OptionRow)
            {
                if(!OptionCache.Contains(OptionRow->OptionID))
                {
                    TArray<FDiaItemOptionRow> NewOptionArray = { *OptionRow };
                    OptionCache.Emplace(OptionRow->OptionID, NewOptionArray);
				}
                else
                {
                    OptionCache[OptionRow->OptionID].Add(*OptionRow);
                }
            }
        }
	}
}

FInventorySlot UItemSubsystem::CreateInventoryInstance(const FName& ItemID, int32 Level, bool bRandomStats)
{
    const FItemBase& ItemData = GetItemData(ItemID);
    FInventorySlot Item;
    Item.ItemInstance.BaseItem = ItemData;
    Item.ItemInstance.Quantity = 1;
    Item.ItemInstance.Level = Level;
    GenerateRandomStats(Item, Level);
    return Item;
}

UItemWidget* UItemSubsystem::CreateItemWidget(const FInventorySlot& Item)
{
    FSoftObjectPath ItemWidgetPath(TEXT("/Game/UI/Inventory/WBP_ItemWidget.WBP_ItemWidget_C"));
    TSoftClassPtr<UUserWidget> WidgetAssetPtr(ItemWidgetPath);
    UClass* ItemWidgetClass = WidgetAssetPtr.LoadSynchronous();


    FSoftObjectPath TooltipWidgetPath(TEXT("/Game/UI/Inventory/WBP_ItemToolTip.WBP_ItemToolTip_C"));
    TSoftClassPtr<UUserWidget> TooltipWidgetAssetPtr(TooltipWidgetPath);
    UClass* TooltipWidgetClass = TooltipWidgetAssetPtr.LoadSynchronous();


    if (ItemWidgetClass)
    {
        UItemWidget* ItemWidget = CreateWidget<UItemWidget>(GetWorld(), ItemWidgetClass);
        if (ItemWidget)
        {
            ItemWidget->SetItemInfo(Item);
            if(TooltipWidgetClass)
            {
                UItemToolTipWidget* ToolTipWidget = CreateWidget<UItemToolTipWidget>(GetWorld(), TooltipWidgetClass);
                if(ToolTipWidget)
                {
                    ItemWidget->SetToolTip(ToolTipWidget);
					ToolTipWidget->SetToolTipItem(Item);
                }
			}
            return ItemWidget;
        }
	}
    return nullptr;
}

void UItemSubsystem::GenerateRandomStats(FInventorySlot& Item, int32 Level)
{
    // 기본 스탯에 추가로 보너스 스탯 생성
}

void UItemSubsystem::GenerateItemOptions(FItemInstance& Item, int32 Level)
{
	int32 NumPrefixOptionsToAdd = 3;
	int32 NumSuffixOptionsToAdd = 3;

	TArray<FDiaItemOptionRow> AvailablePrefixOptions;
    TArray<FDiaItemOptionRow> AvailableSubfixOptions;

    TArray<FDiaItemOptionRow> ResultPrefixOptions;
    TArray<FDiaItemOptionRow> ResultSubfixOptions;

    for(const auto& OptionRows : OptionCache)
    {
		bool bCanApplyOption = true;
        for(const FDiaItemOptionRow& OptionRow : OptionRows.Value)
        {
            //가능 태그 검사
            if (Item.BaseItem.PossibleItemOptionTags.HasAny(OptionRow.RequiredTags))
            {
                bCanApplyOption = true;
            }
            //블록용 태그 검사
            if (Item.BaseItem.PossibleItemOptionTags.HasAny(OptionRow.BlockedTags))
            {
                bCanApplyOption = false;
            }
            //레벨 조건 검사 (아이템)
            if(Level < OptionRow.RequiredItemLevelMin || Level > OptionRow.RequiredItemLevelMax)
            {
                bCanApplyOption = false;
			}
            //추가할 수 없는 옵션 슬롯이라면 거름
            if (Item.GetEquipmentSlot() != EEquipmentSlot::EES_None && !OptionRow.SlotTypes.Contains(Item.GetEquipmentSlot()))
            {
                bCanApplyOption = false;
            }

            //추가할 수 없는 상태
            if (!Item.CheckPrefixOptionsSize() || !Item.CheckPrefixOptionsSize())
            {
                return;
            }

            if (!bCanApplyOption)
            {
                continue;
            }

            //진행
            //가능한 옵션 일단 다 넣고 본다.
            if (OptionRow.OptionType == EItemOptionType::IOT_Prefix)
            {
                AvailablePrefixOptions.Add(OptionRow);
            }
            else if (OptionRow.OptionType == EItemOptionType::IOT_Suffix)
            {
                AvailableSubfixOptions.Add(OptionRow);
			}
        }
	}

    //가중치를 이용해 랜덤한 옵션을 선택해서 넣어준다.
    //우선 모든 가중치를 계산한다.

    

    PickupRandomValuesByWeight<FDiaItemOptionRow>(AvailablePrefixOptions,
        [](const FDiaItemOptionRow& Row) { return static_cast<float>(Row.Weight); },
        NumPrefixOptionsToAdd,
        ResultPrefixOptions);
    PickupRandomValuesByWeight<FDiaItemOptionRow>(AvailableSubfixOptions,
        [](const FDiaItemOptionRow& Row) { return static_cast<float>(Row.Weight); },
        NumSuffixOptionsToAdd,
        ResultSubfixOptions);

    //로그 남기기
	UE_LOG(LogTemp, Warning, TEXT("Generated Prefix Options:"));
    for (const FDiaItemOptionRow& Option : ResultPrefixOptions)
    {
        UE_LOG(LogTemp, Warning, TEXT(" - %s (Tier %d)"), *Option.DisplayName.ToString(), Option.TierIndex);
    }
    UE_LOG(LogTemp, Warning, TEXT("Generated Suffix Options:"));
    for (const FDiaItemOptionRow& Option : ResultSubfixOptions)
    {
        UE_LOG(LogTemp, Warning, TEXT(" - %s (Tier %d)"), *Option.DisplayName.ToString(), Option.TierIndex);
	}
}

const FItemBase& UItemSubsystem::GetItemData(const FName& ItemID) const
{
    const FItemBase* FoundItem = ItemCache.Find(ItemID);

    if (!FoundItem)
    {
        if (ItemDataTable)
        {
            FoundItem = ItemDataTable->FindRow<FItemBase>(ItemID, TEXT(""));
            if (FoundItem)
            {
                ItemCache.Emplace(FoundItem->ItemID, *FoundItem);
                return *FoundItem;
            }
        }
#ifdef UE_BUILD_DEBUG
        UE_LOG(LogTemp, Warning, TEXT("ItemSubsystem: Item not found with ID: %s"), *ItemID.ToString());
#endif
        static const FItemBase DefaultItem;
        return DefaultItem;
    }

    return *FoundItem;
}