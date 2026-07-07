// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ItemSubsystem.h"
#include "UI/Item/ItemWidget.h"
#include "UI/Item/ItemToolTipWidget.h"
#include "Types/ItemBase.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"
#include "Logging/ARPGLogChannels.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
	LoadItemData();
    LoadOptionData();
    LoadDropData();

    UE_LOG(LogARPG, Warning, TEXT("ItemSubsystem: LoadComplete! "));
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
		UE_LOG(LogARPG, Warning, TEXT("ItemSubsystem: Failed to load item data table from path: %s"), *ItemDataTablePath);
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
        UE_LOG(LogARPG, Warning, TEXT("ItemSubsystem: Failed to load item data table from path: %s"), *OptionDataTablePath);
    }
#endif

}

void UItemSubsystem::LoadDropData()
{
    UDataTable* DropDataTable = LoadObject<UDataTable>(nullptr, *DropDataTablePath);
    if (DropDataTable)
    {
        DropCache.Empty();
        TArray<FName> RowNames = DropDataTable->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            FMonsterDropTable* DropRow = DropDataTable->FindRow<FMonsterDropTable>(RowName, TEXT(""));
            if (DropRow )
            {
                DropCache.Emplace(DropRow->MonsterID, *DropRow);
            }
        }
    }
}

UItemWidget* UItemSubsystem::CreateItemWidget(UWorld* WorldContext, const FInventorySlot& Item)
{
    FSoftObjectPath ItemWidgetPath(TEXT("/Game/UI/Inventory/WBP_ItemWidget.WBP_ItemWidget_C"));
    TSoftClassPtr<UUserWidget> WidgetAssetPtr(ItemWidgetPath);
    UClass* ItemWidgetClass = WidgetAssetPtr.LoadSynchronous();

    if (ItemWidgetClass)
    {
        UItemWidget* ItemWidget = CreateWidget<UItemWidget>(WorldContext, ItemWidgetClass);
        if (ItemWidget)
        {
            ItemWidget->SetItemInfo(Item);
            return ItemWidget;
        }
	}

    return nullptr;
}

UItemWidget* UItemSubsystem::CreateItemWidget(UUserWidget* WidgetContext, const FInventorySlot& Item)
{
    FSoftObjectPath ItemWidgetPath(TEXT("/Game/UI/Inventory/WBP_ItemWidget.WBP_ItemWidget_C"));
    TSoftClassPtr<UUserWidget> WidgetAssetPtr(ItemWidgetPath);
    UClass* ItemWidgetClass = WidgetAssetPtr.LoadSynchronous();

    if (ItemWidgetClass)
    {
        UItemWidget* ItemWidget = CreateWidget<UItemWidget>(WidgetContext, ItemWidgetClass);
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

TArray<FItemDropInfo> UItemSubsystem::GetRandomDropItem(const FName& MonsterID) const
{
    if(FMonsterDropTable* DropInfo = DropCache.Find(MonsterID))
    {
        TArray<FItemDropInfo> DropList = DropInfo->GetRandomizedDropItems();

		return DropList;
    }

    return TArray<FItemDropInfo>();
}

void UItemSubsystem::GenerateRandomStats(FItemInstance& Item, int32 Level)
{
    // 湲곕낯 ?ㅽ꺈??異붽?濡?蹂대꼫???ㅽ꺈 ?앹꽦
}

const FDiaItemOptionRow* UItemSubsystem::FindOptionRowByKey(const FName& OptionKey) const
{
    if (const FDiaItemOptionRow* FoundOption = OptionCache.Find(OptionKey))
    {
        return FoundOption;
    }

    for (const auto& OptionPair : OptionCache)
    {
        if (OptionPair.Value.OptionID == OptionKey)
        {
            return &OptionPair.Value;
        }
    }

    return nullptr;
}

bool UItemSubsystem::CanApplyOptionToItem(
    const FItemInstance& Item,
    const FDiaItemOptionRow& OptionRow,
    EItemOptionType ExpectedOptionType,
    int32 Level,
    FString* OutFailureReason) const
{
    if (OptionRow.OptionType != ExpectedOptionType)
    {
        if (OutFailureReason)
        {
            *OutFailureReason = TEXT("option type mismatch");
        }
        return false;
    }

    if (Level < OptionRow.RequiredItemLevelMin || Level > OptionRow.RequiredItemLevelMax)
    {
        if (OutFailureReason)
        {
            *OutFailureReason = TEXT("item level is outside option range");
        }
        return false;
    }

	const FItemBase* ItemData = FindItemData(Item);
    if (!ItemData)
    {
        if (OutFailureReason)
        {
            *OutFailureReason = FString::Printf(TEXT("item data not found: %s"), *Item.ItemID.ToString());
        }
        return false;
    }

    const EEquipmentSlot EquipmentSlot = ItemData->EquipmentSlot;
    if (EquipmentSlot == EEquipmentSlot::EES_None || !OptionRow.SlotTypes.Contains(EquipmentSlot))
    {
        if (OutFailureReason)
        {
            *OutFailureReason = TEXT("equipment slot is not allowed");
        }
        return false;
    }

    for (const FGameplayTag& RequiredTag : OptionRow.RequiredTags)
    {
        if (!ItemData->PossibleItemOptionTags.HasTag(RequiredTag))
        {
            if (OutFailureReason)
            {
                *OutFailureReason = FString::Printf(TEXT("missing required tag: %s"), *RequiredTag.ToString());
            }
            return false;
        }
    }

    for (const FGameplayTag& BlockedTag : OptionRow.BlockedTags)
    {
        if (ItemData->PossibleItemOptionTags.HasTag(BlockedTag))
        {
            if (OutFailureReason)
            {
                *OutFailureReason = FString::Printf(TEXT("blocked tag exists: %s"), *BlockedTag.ToString());
            }
            return false;
        }
    }

    return true;
}

bool UItemSubsystem::TryAddItemOptionByKey(
    FItemInstance& Item,
    const FName& OptionKey,
    EItemOptionType ExpectedOptionType,
    int32 Level,
    FString* OutFailureReason) const
{
    if (OptionKey.IsNone())
    {
        return true;
    }

    const FDiaItemOptionRow* OptionRow = FindOptionRowByKey(OptionKey);
    if (!OptionRow)
    {
        if (OutFailureReason)
        {
            *OutFailureReason = FString::Printf(TEXT("option not found: %s"), *OptionKey.ToString());
        }
        return false;
    }

    if (!CanApplyOptionToItem(Item, *OptionRow, ExpectedOptionType, Level, OutFailureReason))
    {
        return false;
    }

    FDiaActualItemOption ActualOption(*OptionRow);
    if (ExpectedOptionType == EItemOptionType::IOT_Prefix)
    {
        if (!Item.CheckPrefixOptionsSize())
        {
            if (OutFailureReason)
            {
                *OutFailureReason = TEXT("prefix option slots are full");
            }
            return false;
        }

        Item.PrefixOptions.Add(ActualOption);
        return true;
    }

    if (!Item.CheckSuffixOptionsSize())
    {
        if (OutFailureReason)
        {
            *OutFailureReason = TEXT("suffix option slots are full");
        }
        return false;
    }

    Item.SuffixOptions.Add(ActualOption);
    return true;
}

void UItemSubsystem::GenerateItemOptions(FItemInstance& Item, int32 Level)
{
    const int32 NumPrefixOptionsToAdd = MAX_PREFIX_OPTIONS - Item.PrefixOptions.Num();
    const int32 NumSuffixOptionsToAdd = MAX_SUFFIX_OPTIONS - Item.SuffixOptions.Num();  

    if (NumPrefixOptionsToAdd <= 0 && NumSuffixOptionsToAdd <= 0)
    {
        return;
    }

#if UE_EDITOR
    UE_LOG(LogARPG, Warning, TEXT("GenerateItemOptions: Start | ItemID: %s, Level: %d"),
        *Item.ItemID.ToString(), Level);
    UE_LOG(LogARPG, Warning, TEXT("GenerateItemOptions: OptionCache Num: %d"), OptionCache.Num());
#endif

    const FItemBase* ItemData = FindItemData(Item);
    if (!ItemData)
    {
        UE_LOG(LogARPG, Warning, TEXT("GenerateItemOptions: item data not found. ItemID: %s"), *Item.ItemID.ToString());
        return;
    }

    const FGameplayTagContainer& PossibleOptionTags = ItemData->PossibleItemOptionTags;
    const EEquipmentSlot EquipmentSlot = ItemData->EquipmentSlot;

	TArray<FDiaItemOptionRow> AvailablePrefixOptions;
    TArray<FDiaItemOptionRow> AvailableSuffixOptions;

    TArray<FDiaItemOptionRow> ResultPrefixOptions;
    TArray<FDiaItemOptionRow> ResultSuffixOptions;

    for(const auto& OptionRows : OptionCache)
    {
		bool bCanApplyOption = true;
        const FDiaItemOptionRow& OptionRow = OptionRows.Value;

        for (const FGameplayTag& RequiredTag : OptionRow.RequiredTags)
        {
            if (!PossibleOptionTags.HasTag(RequiredTag))
            {
                bCanApplyOption = false;
            }
        }

        for (const FGameplayTag& BlockedTag : OptionRow.BlockedTags)
        {
            if (PossibleOptionTags.HasTag(BlockedTag))
            {
                bCanApplyOption = false;
            }
        }

		if (Level < OptionRow.RequiredItemLevelMin || Level > OptionRow.RequiredItemLevelMax)
		{
			bCanApplyOption = false;
		}

        if (EquipmentSlot == EEquipmentSlot::EES_None || !OptionRow.SlotTypes.Contains(EquipmentSlot))
        {
            bCanApplyOption = false;
        }

        if (!bCanApplyOption)
        {
            continue;
        }

        if (OptionRow.OptionType == EItemOptionType::IOT_Prefix)
        {
            AvailablePrefixOptions.Add(OptionRow);
        }
        else if (OptionRow.OptionType == EItemOptionType::IOT_Suffix)
        {
            AvailableSuffixOptions.Add(OptionRow);
		}
#if UE_EDITOR
        UE_LOG(LogARPG, Warning, TEXT("OptionID: %s"), *OptionRow.OptionID.ToString());
#endif // UE_EDITOR
	}


    PickupRandomValuesByWeight<FDiaItemOptionRow>(AvailablePrefixOptions,
        [](const FDiaItemOptionRow& Row) { return static_cast<float>(Row.Weight); },
        NumPrefixOptionsToAdd,
        ResultPrefixOptions);
    PickupRandomValuesByWeight<FDiaItemOptionRow>(AvailableSuffixOptions,
        [](const FDiaItemOptionRow& Row) { return static_cast<float>(Row.Weight); },    
        NumSuffixOptionsToAdd,
        ResultSuffixOptions);

    //?듭뀡???ｌ뼱以??    //?대??곸쑝濡?makerandomvalue瑜??몄텧?쒕떎.
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

    //濡쒓렇 ?④린湲?	//UE_LOG(LogARPG, Warning, TEXT("Generated Prefix Options:"));
 //   for (const FDiaItemOptionRow& Option : ResultPrefixOptions)
 //   {
 //       UE_LOG(LogARPG, Warning, TEXT(" - %s (Tier %d)"), *Option.DisplayName.ToString(), Option.TierIndex);
 //       if (Option.OptionType != EItemOptionType::IOT_Prefix)
 //       {
 //           UE_LOG(LogARPG, Warning, TEXT("Error: Generated option is not Prefix type! OptionID: %s"), *Option.OptionID.ToString());
	//	}
 //       if (Option.GrantedTag.IsValid() == false)
 //       {
 //           UE_LOG(LogARPG, Warning, TEXT("Error: Generated option has invalid GrantedTag! OptionID: %s"), *Option.OptionID.ToString());
 //       }
 //   }
 //   UE_LOG(LogARPG, Warning, TEXT("Generated Suffix Options:"));
 //   for (const FDiaItemOptionRow& Option : ResultSuffixOptions)
 //   {
 //       UE_LOG(LogARPG, Warning, TEXT(" - %s (Tier %d)"), *Option.DisplayName.ToString(), Option.TierIndex);
 //       if (Option.OptionType != EItemOptionType::IOT_Prefix)
 //       {
 //           UE_LOG(LogARPG, Warning, TEXT("Error: Generated option is not Suffix type! OptionID: %s"), *Option.OptionID.ToString());
 //       }
 //       if (Option.GrantedTag.IsValid() == false)
 //       {
 //           UE_LOG(LogARPG, Warning, TEXT("Error: Generated option has invalid GrantedTag! OptionID: %s"), *Option.OptionID.ToString());
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
        UE_LOG(LogARPG, Warning, TEXT("ItemSubsystem: Item not found with ID: %s"), *ItemID.ToString());
#endif
        static const FItemBase DefaultItem;
        return DefaultItem;
    }

    return *FoundItem;
}

const FItemBase *UItemSubsystem::FindItemData(const FName &ItemID) const
{
    const FItemBase& ItemData = GetItemData(ItemID);
    return ItemData.ItemID.IsNone() ? nullptr : &ItemData;
}

const FItemBase* UItemSubsystem::FindItemData(const FItemInstance& Item) const
{
    return FindItemData(Item.ItemID);
}

FText UItemSubsystem::GetItemDisplayName(const FItemInstance& Item) const
{
    if (const FItemBase* ItemData = FindItemData(Item))
    {
        return ItemData->Name;
    }

    return FText::FromName(Item.ItemID);
}

int32 UItemSubsystem::GetItemWidth(const FItemInstance& Item) const
{
    if (const FItemBase* ItemData = FindItemData(Item))
    {
        return ItemData->Width;
    }

    return 1;
}

int32 UItemSubsystem::GetItemHeight(const FItemInstance& Item) const
{
    if (const FItemBase* ItemData = FindItemData(Item))
    {
        return ItemData->Height;
    }

    return 1;
}

EEquipmentSlot UItemSubsystem::GetEquipmentSlot(const FItemInstance& Item) const
{
    if (const FItemBase* ItemData = FindItemData(Item))
    {
        return ItemData->EquipmentSlot;
    }

    return EEquipmentSlot::EES_None;
}

FSoftObjectPath UItemSubsystem::GetIconPath(const FItemInstance& Item) const
{
    if (const FItemBase* ItemData = FindItemData(Item))
    {
        return ItemData->IconPath;
    }

    return FSoftObjectPath();
}

bool UItemSubsystem::IsItemStackable(const FItemInstance& Item) const
{
    if (const FItemBase* ItemData = FindItemData(Item))
    {
        return ItemData->bStackable;
    }

    return false;
}

const FGameplayTagContainer* UItemSubsystem::GetPossibleItemOptionTags(const FItemInstance& Item) const
{
    if (const FItemBase* ItemData = FindItemData(Item))
    {
        return &ItemData->PossibleItemOptionTags;
    }

    return nullptr;
}

void UItemSubsystem::CreateInventoryInstance(FInventorySlot &OutItem, const FName &ItemID, int32 Level, bool bRandomStats, int32 Quantity)
{
    const FItemBase& ItemData = GetItemData(ItemID);
    OutItem = FInventorySlot::FromBase(ItemData, FMath::Max(Quantity, 1));
    OutItem.ItemInstance.Quantity = FMath::Max(Quantity, 1);
	OutItem.ItemInstance.Level = Level;
    GenerateRandomStats(OutItem.ItemInstance, Level);
    GenerateItemOptions(OutItem.ItemInstance, Level);
}

void UItemSubsystem::CreateInventoryInstanceByItemBase(FInventorySlot& OutItem, const FItemBase& ItemData, int32 Level, bool bRandomStats, int32 Quantity)
{
	OutItem = FInventorySlot::FromBase(ItemData, FMath::Max(Quantity, 1));
    OutItem.ItemInstance.Quantity = FMath::Max(Quantity, 1);
    OutItem.ItemInstance.Level = Level;
    GenerateRandomStats(OutItem.ItemInstance, Level);
    GenerateItemOptions(OutItem.ItemInstance, Level);
}

void UItemSubsystem::CreateInventoryInstanceWithOptions(
    FInventorySlot& OutItem,
    FName& ItemID,
    int32 Level,
    const TArray<FName>& PrefixOptionKeys,
    const TArray<FName>& SuffixOptionKeys,
    int32 Quantity)
{
    const FItemBase& ItemData = GetItemData(ItemID);
    const int32 SafeQuantity = FMath::Max(Quantity, 1);
    const int32 SafeLevel = FMath::Max(Level, 1);

    OutItem = FInventorySlot::FromBase(ItemData, SafeQuantity);
    OutItem.ItemInstance.Quantity = SafeQuantity;
    OutItem.ItemInstance.Level = SafeLevel;
    GenerateRandomStats(OutItem.ItemInstance, SafeLevel);

    for (const FName& PrefixOptionKey : PrefixOptionKeys)
    {
        FString FailureReason;
        if (!TryAddItemOptionByKey(OutItem.ItemInstance, PrefixOptionKey, EItemOptionType::IOT_Prefix, SafeLevel, &FailureReason))
        {
            UE_LOG(LogARPG, Warning, TEXT("CreateInventoryInstanceWithOptions: prefix option '%s' skipped for item '%s': %s"),
                *PrefixOptionKey.ToString(),
                *ItemID.ToString(),
                *FailureReason);
        }
    }

    for (const FName& SuffixOptionKey : SuffixOptionKeys)
    {
        FString FailureReason;
        if (!TryAddItemOptionByKey(OutItem.ItemInstance, SuffixOptionKey, EItemOptionType::IOT_Suffix, SafeLevel, &FailureReason))
        {
            UE_LOG(LogARPG, Warning, TEXT("CreateInventoryInstanceWithOptions: suffix option '%s' skipped for item '%s': %s"),
                *SuffixOptionKey.ToString(),
                *ItemID.ToString(),
                *FailureReason);
        }
    }
}

const FString& UItemSubsystem::RunOptionRollStressTest(FName ItemID, int32 Count, int32 Level)
{
    const int32 SafeCount = FMath::Clamp(Count, 1, 10000);
    const int32 SafeLevel = FMath::Max(Level, 1);

    const FItemBase& ItemData = GetItemData(ItemID);
    if (ItemData.ItemID.IsNone())
    {
        LastOptionRollStressReport = FString::Printf(
            TEXT("== Option Roll Stress Test ==\nItem=%s\nResult=FAIL\nReason=item data not found\n"),
            *ItemID.ToString());
        UE_LOG(LogARPG_Inventory, Warning, TEXT("%s"), *LastOptionRollStressReport);
        return LastOptionRollStressReport;
    }

    int32 EmptyPrefixCount = 0;
    int32 EmptySuffixCount = 0;
    int32 DuplicateOptionCount = 0;
    int32 InvalidSlotCount = 0;
    int32 EmptyGrantedTagCount = 0;
    int32 AttributeMappingFailCount = 0;
    int32 MissingOptionRowCount = 0;

    TMap<FName, int32> PrefixDistribution;
    TMap<FName, int32> SuffixDistribution;
    TArray<FString> Samples;

    auto ShouldRequireAttributeMapping = [](const FGameplayTag& GrantedTag)
    {
        const FDiaGameplayTags& Tags = FDiaGameplayTags::Get();
        return GrantedTag != Tags.ItemOptionLifeSteal
            && GrantedTag != Tags.ItemOptionGoldFind;
    };

    auto ValidateRolledOptions = [&](
        const FInventorySlot& RolledItem,
        const TArray<FDiaActualItemOption>& Options,
        EItemOptionType ExpectedType,
        TMap<FName, int32>& Distribution)
    {
        TSet<FName> SeenUniqueKeys;
        for (const FDiaActualItemOption& Option : Options)
        {
            const FName UniqueKey = Option.GetUniqueOptionKey();
            Distribution.FindOrAdd(UniqueKey)++;

            if (SeenUniqueKeys.Contains(UniqueKey))
            {
                ++DuplicateOptionCount;
                if (Samples.Num() < 12)
                {
                    Samples.Add(FString::Printf(TEXT("DUPLICATE Option=%s"), *UniqueKey.ToString()));
                }
            }
            SeenUniqueKeys.Add(UniqueKey);

            if (!Option.GrantedTag.IsValid())
            {
                ++EmptyGrantedTagCount;
                if (Samples.Num() < 12)
                {
                    Samples.Add(FString::Printf(TEXT("EMPTY_GRANTED_TAG Option=%s"), *UniqueKey.ToString()));
                }
                continue;
            }

            const FDiaItemOptionRow* OptionRow = FindOptionRowByKey(UniqueKey);
            if (!OptionRow)
            {
                OptionRow = FindOptionRowByKey(Option.OptionID);
            }

            if (!OptionRow)
            {
                ++MissingOptionRowCount;
                if (Samples.Num() < 12)
                {
                    Samples.Add(FString::Printf(TEXT("MISSING_OPTION_ROW Option=%s"), *UniqueKey.ToString()));
                }
            }
            else
            {
                const EEquipmentSlot EquipmentSlot = GetEquipmentSlot(RolledItem.ItemInstance);
                if (OptionRow->OptionType != ExpectedType || !OptionRow->SlotTypes.Contains(EquipmentSlot))
                {
                    ++InvalidSlotCount;
                    if (Samples.Num() < 12)
                    {
                        Samples.Add(FString::Printf(TEXT("INVALID_SLOT Item=%s Slot=%s Option=%s"),
                            *RolledItem.ItemInstance.ItemID.ToString(),
                            *UEnum::GetValueAsString(EquipmentSlot),
                            *UniqueKey.ToString()));
                    }
                }
            }

            FGameplayAttribute Attribute;
            if (ShouldRequireAttributeMapping(Option.GrantedTag)
                && !UDiaAttributeSet::TranslateAttributeTagToAttribute(Option.GrantedTag, Attribute))
            {
                ++AttributeMappingFailCount;
                if (Samples.Num() < 12)
                {
                    Samples.Add(FString::Printf(TEXT("ATTRIBUTE_MAPPING_FAILED Option=%s Tag=%s"),
                        *UniqueKey.ToString(),
                        *Option.GrantedTag.ToString()));
                }
            }
        }
    };

    for (int32 Index = 0; Index < SafeCount; ++Index)
    {
        FInventorySlot RolledItem;
        FName ItemIDCopy = ItemID;
        CreateInventoryInstance(RolledItem, ItemIDCopy, SafeLevel, true);

        if (RolledItem.ItemInstance.PrefixOptions.Num() == 0)
        {
            ++EmptyPrefixCount;
        }
        if (RolledItem.ItemInstance.SuffixOptions.Num() == 0)
        {
            ++EmptySuffixCount;
        }

        ValidateRolledOptions(
            RolledItem,
            RolledItem.ItemInstance.PrefixOptions,
            EItemOptionType::IOT_Prefix,
            PrefixDistribution);
        ValidateRolledOptions(
            RolledItem,
            RolledItem.ItemInstance.SuffixOptions,
            EItemOptionType::IOT_Suffix,
            SuffixDistribution);
    }

    const int32 ErrorCount = DuplicateOptionCount
        + InvalidSlotCount
        + EmptyGrantedTagCount
        + AttributeMappingFailCount
        + MissingOptionRowCount;

    auto AppendDistribution = [](FString& Report, const TCHAR* Label, const TMap<FName, int32>& Distribution)
    {
        Report += FString::Printf(TEXT("\n[%s]\n"), Label);
        if (Distribution.Num() == 0)
        {
            Report += TEXT("None\n");
            return;
        }

        TArray<TPair<FName, int32>> Sorted;
        for (const auto& Pair : Distribution)
        {
            Sorted.Add(TPair<FName, int32>(Pair.Key, Pair.Value));
        }
        Sorted.Sort([](const TPair<FName, int32>& A, const TPair<FName, int32>& B)
        {
            return A.Value > B.Value;
        });

        for (const TPair<FName, int32>& Pair : Sorted)
        {
            Report += FString::Printf(TEXT("%s: %d\n"), *Pair.Key.ToString(), Pair.Value);
        }
    };

    LastOptionRollStressReport = FString::Printf(
        TEXT("== Option Roll Stress Test ==\nItem=%s\nCount=%d\nLevel=%d\nResult=%s\n\n[Summary]\nEmptyPrefix=%d\nEmptySuffix=%d\nDuplicateOption=%d\nInvalidSlot=%d\nEmptyGrantedTag=%d\nAttributeMappingFailed=%d\nMissingOptionRow=%d\n"),
        *ItemID.ToString(),
        SafeCount,
        SafeLevel,
        ErrorCount == 0 ? TEXT("PASS") : TEXT("FAIL"),
        EmptyPrefixCount,
        EmptySuffixCount,
        DuplicateOptionCount,
        InvalidSlotCount,
        EmptyGrantedTagCount,
        AttributeMappingFailCount,
        MissingOptionRowCount);

    if (Samples.Num() > 0)
    {
        LastOptionRollStressReport += TEXT("\n[Samples]\n");
        for (const FString& Sample : Samples)
        {
            LastOptionRollStressReport += Sample + TEXT("\n");
        }
    }

    AppendDistribution(LastOptionRollStressReport, TEXT("Prefix Distribution"), PrefixDistribution);
    AppendDistribution(LastOptionRollStressReport, TEXT("Suffix Distribution"), SuffixDistribution);

    UE_LOG(LogARPG_Inventory, Display, TEXT("%s"), *LastOptionRollStressReport);
    return LastOptionRollStressReport;
}
