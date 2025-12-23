// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ItemSubsystem.h"
#include "UI/Item/ItemWidget.h"
#include "UI/Item/ItemToolTipWidget.h"
#include "Types/ItemBase.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
	LoadItemData();
    LoadOptionData();
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
#ifdef UE_EDITOR
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
            if (OptionRow && OptionRow->GrantedTag.IsValid())
            {
				FName UniqueOptionKey = FName(*FString::Printf(TEXT("%s_Tier%d"), *OptionRow->OptionID.ToString(), OptionRow->TierIndex));
                OptionCache.Emplace(UniqueOptionKey, *OptionRow);
            }
        }
	}
#ifdef UE_EDITOR
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ItemSubsystem: Failed to load item data table from path: %s"), *OptionDataTablePath);
    }
#endif

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

UItemWidget* UItemSubsystem::CreateItemWidgetEmpty()
{
    FSoftObjectPath ItemWidgetPath(TEXT("/Game/UI/Inventory/WBP_ItemWidget.WBP_ItemWidget_C"));
    TSoftClassPtr<UUserWidget> WidgetAssetPtr(ItemWidgetPath);
    UClass* ItemWidgetClass = WidgetAssetPtr.LoadSynchronous();

    if (ItemWidgetClass)
    {
        UItemWidget* ItemWidget = CreateWidget<UItemWidget>(GetWorld(), ItemWidgetClass);
        return ItemWidget;
    }

    return nullptr;
}

void UItemSubsystem::GenerateRandomStats(FItemInstance& Item, int32 Level)
{
    // 기본 스탯에 추가로 보너스 스탯 생성
}

void UItemSubsystem::GenerateItemOptions(FItemInstance& Item, int32 Level)
{
	int32 NumPrefixOptionsToAdd = 3;
	int32 NumSuffixOptionsToAdd = 3;

#if UE_EDITOR
    UE_LOG(LogTemp, Warning, TEXT("GenerateItemOptions: Start | ItemID: %s, Level: %d"),
        *Item.BaseItem.ItemID.ToString(), Level);
    UE_LOG(LogTemp, Warning, TEXT("GenerateItemOptions: OptionCache Num: %d"), OptionCache.Num());
#endif

	TArray<FDiaItemOptionRow> AvailablePrefixOptions;
    TArray<FDiaItemOptionRow> AvailableSuffixOptions;

    TArray<FDiaItemOptionRow> ResultPrefixOptions;
    TArray<FDiaItemOptionRow> ResultSuffixOptions;

    for(const auto& OptionRows : OptionCache)
    {
		bool bCanApplyOption = true;
        const FDiaItemOptionRow& OptionRow = OptionRows.Value;
        {
            //추가할 수 없는 상태
            if (!Item.CheckPrefixOptionsSize() || !Item.CheckSuffixOptionsSize())
            {
                return;
            }

            //가능 태그 검사
            for (const auto& RequireTag : OptionRow.RequiredTags)
            {
                if (!Item.BaseItem.PossibleItemOptionTags.HasTag(RequireTag))
                {
                    bCanApplyOption = false;
                }
            }


            //불가능 태그 검사 (이거 있으면 생성 안해줌)
            for (const auto& BlockTag : OptionRow.BlockedTags)
            {
                if (Item.BaseItem.PossibleItemOptionTags.HasTag(BlockTag))
                {
                    bCanApplyOption = false;
                }
            }

			//레벨 조건 검사(아이템) HACK. 디버그 용으로 최소 레벨을 0으로 둠
			if (Level < /*OptionRow.RequiredItemLevelMin*/ 0 || Level > OptionRow.RequiredItemLevelMax)
			{
				bCanApplyOption = false;
			}

            //추가할 수 없는 옵션 슬롯이라면 거름
            if (Item.GetEquipmentSlot() != EEquipmentSlot::EES_None && !OptionRow.SlotTypes.Contains(Item.GetEquipmentSlot()))
            {
                bCanApplyOption = false;
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
                AvailableSuffixOptions.Add(OptionRow);
			}
#if UE_EDITOR
            UE_LOG(LogTemp, Warning, TEXT("OptionID: %s"), *OptionRow.OptionID.ToString());
#endif // UE_EDITOR

        }
	}

    PickupRandomValuesByWeight<FDiaItemOptionRow>(AvailablePrefixOptions,
        [](const FDiaItemOptionRow& Row) { return static_cast<float>(Row.Weight); },
        NumPrefixOptionsToAdd,
        ResultPrefixOptions);
    PickupRandomValuesByWeight<FDiaItemOptionRow>(AvailableSuffixOptions,
        [](const FDiaItemOptionRow& Row) { return static_cast<float>(Row.Weight); },
        NumSuffixOptionsToAdd,
        ResultSuffixOptions);

    //옵션을 넣어준다
    //내부적으로 makerandomvalue를 호출한다.
    for (const FDiaItemOptionRow& OptionRow : ResultPrefixOptions)
    {
        FDiaActualItemOption ActualOption(OptionRow);
        Item.PrefixOptions.Add(ActualOption);
    }
    for (const FDiaItemOptionRow& OptionRow : ResultSuffixOptions)
    {
        FDiaActualItemOption ActualOption(OptionRow);
        Item.SuffixOptions.Add(ActualOption);
	}

    //로그 남기기
	//UE_LOG(LogTemp, Warning, TEXT("Generated Prefix Options:"));
 //   for (const FDiaItemOptionRow& Option : ResultPrefixOptions)
 //   {
 //       UE_LOG(LogTemp, Warning, TEXT(" - %s (Tier %d)"), *Option.DisplayName.ToString(), Option.TierIndex);
 //       if (Option.OptionType != EItemOptionType::IOT_Prefix)
 //       {
 //           UE_LOG(LogTemp, Warning, TEXT("Error: Generated option is not Prefix type! OptionID: %s"), *Option.OptionID.ToString());
	//	}
 //       if (Option.GrantedTag.IsValid() == false)
 //       {
 //           UE_LOG(LogTemp, Warning, TEXT("Error: Generated option has invalid GrantedTag! OptionID: %s"), *Option.OptionID.ToString());
 //       }
 //   }
 //   UE_LOG(LogTemp, Warning, TEXT("Generated Suffix Options:"));
 //   for (const FDiaItemOptionRow& Option : ResultSuffixOptions)
 //   {
 //       UE_LOG(LogTemp, Warning, TEXT(" - %s (Tier %d)"), *Option.DisplayName.ToString(), Option.TierIndex);
 //       if (Option.OptionType != EItemOptionType::IOT_Prefix)
 //       {
 //           UE_LOG(LogTemp, Warning, TEXT("Error: Generated option is not Suffix type! OptionID: %s"), *Option.OptionID.ToString());
 //       }
 //       if (Option.GrantedTag.IsValid() == false)
 //       {
 //           UE_LOG(LogTemp, Warning, TEXT("Error: Generated option has invalid GrantedTag! OptionID: %s"), *Option.OptionID.ToString());
 //       }
	//}
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
#ifdef UE_EDITOR
        UE_LOG(LogTemp, Warning, TEXT("ItemSubsystem: Item not found with ID: %s"), *ItemID.ToString());
#endif
        static const FItemBase DefaultItem;
        return DefaultItem;
    }

    return *FoundItem;
}

void UItemSubsystem::CreateInventoryInstance(FInventorySlot& OutItem, FName& ItemID, int32 Level, bool bRandomStats)
{
    const FItemBase& ItemData = GetItemData(ItemID);
    OutItem = FInventorySlot::FromBase(ItemData, 1);
    OutItem.ItemInstance.Quantity = 1;
	OutItem.ItemInstance.Level = Level;
    GenerateRandomStats(OutItem.ItemInstance, Level);
    GenerateItemOptions(OutItem.ItemInstance, Level);
}

void UItemSubsystem::CreateInventoryInstanceByItemBase(FInventorySlot& OutItem, const FItemBase& ItemData, int32 Level, bool bRandomStats)
{
	OutItem = FInventorySlot::FromBase(ItemData, 1);
    OutItem.ItemInstance.Quantity = 1;
    OutItem.ItemInstance.Level = Level;
    GenerateRandomStats(OutItem.ItemInstance, Level);
    GenerateItemOptions(OutItem.ItemInstance, Level);
}
