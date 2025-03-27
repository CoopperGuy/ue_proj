// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ItemSubsystem.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
	LoadItemData();
    UE_LOG(LogTemp, Warning, TEXT("ItemSubsystem: LoadComplete! "));

}

void UItemSubsystem::Deinitialize()
{
    Super::Deinitialize();
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

//아이템 드롭 시 인스턴스 사용.
//아이템 내용 저장해야할거같은데.
FInventoryItem UItemSubsystem::CreateItemInstance(const FName& ItemID, int32 Level, bool bRandomStats)
{
    const FItemBase& ItemData = GetItemData(ItemID);
    FInventoryItem Item;
    Item.ItemID = ItemData.ItemID;
    Item.Quantity = 1;
    Item.Level = Level;
    Item.bRandomStats = bRandomStats;
    GenerateRandomStats(Item, Level);
    return Item;
}

void UItemSubsystem::GenerateRandomStats(FInventoryItem& Item, int32 Level)
{
    if (!Item.bRandomStats)
    {
        return;
    }  

    Item.Stats.Empty();
    Item.Stats.Add(EItemStat::EIS_Health, FMath::RandRange(10, 20));
    Item.Stats.Add(EItemStat::EIS_Mana, FMath::RandRange(10, 20));
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