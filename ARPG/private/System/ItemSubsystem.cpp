// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ItemSubsystem.h"
#include "UI/Item/ItemWidget.h"
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

FInventorySlot UItemSubsystem::CreateItemInstance(const FName& ItemID, int32 Level, bool bRandomStats)
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

    if (ItemWidgetClass)
    {
        UItemWidget* ItemWidget = CreateWidget<UItemWidget>(GetWorld(), ItemWidgetClass);
        if (ItemWidget)
        {
            ItemWidget->SetItemInfo(Item);
            return ItemWidget;
        }
	}
    return nullptr;
}

void UItemSubsystem::GenerateRandomStats(FInventorySlot& Item, int32 Level)
{
    // 기본 스탯에 추가로 보너스 스탯 생성
    Item.ItemInstance.BonusStats.Empty();
    Item.ItemInstance.BonusStats.Add(EItemStat::EIS_Health, FMath::RandRange(5, 15) * Level);
    Item.ItemInstance.BonusStats.Add(EItemStat::EIS_Mana, FMath::RandRange(5, 15) * Level);
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