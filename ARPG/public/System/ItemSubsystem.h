// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/ItemBase.h"  
#include "Types/DiaItemOptionRow.h"
#include "Types/DiaDropTable.h"
#include "ItemSubsystem.generated.h"

class UItemWidget;
/**
 * 
 */
UCLASS()
class ARPG_API UItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    // USubsystem 
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    TArray<FName> GetRewardEquipmentIDs(bool bCommonOnly = false) const;
    const FItemBase& GetItemData(const FName& ItemID) const;
    const FItemBase* FindItemData(const FName& ItemID) const;
    const FItemBase* FindItemData(const FItemInstance& Item) const;
    FText GetItemDisplayName(const FItemInstance& Item) const;
    int32 GetItemWidth(const FItemInstance& Item) const;
    int32 GetItemHeight(const FItemInstance& Item) const;
    EEquipmentSlot GetEquipmentSlot(const FItemInstance& Item) const;
    FSoftObjectPath GetIconPath(const FItemInstance& Item) const;
    bool IsItemStackable(const FItemInstance& Item) const;
    const FGameplayTagContainer* GetPossibleItemOptionTags(const FItemInstance& Item) const;
    
    void CreateInventoryInstance(FInventorySlot& OutItem, const FName& ItemID, int32 Level = 1, bool bRandomStats = false, int32 Quantity = 1);
    void CreateInventoryInstanceByItemBase(FInventorySlot& OutItem, const FItemBase& ItemData, int32 Level = 1, bool bRandomStats = false, int32 Quantity = 1);
    void CreateInventoryInstanceWithOptions(
        FInventorySlot& OutItem,
        FName& ItemID,
        int32 Level,
        const TArray<FName>& PrefixOptionKeys,
        const TArray<FName>& SuffixOptionKeys,
        int32 Quantity = 1);

    bool TryAddItemOptionByKey(
        FItemInstance& Item,
        const FName& OptionKey,
        EItemOptionType ExpectedOptionType,
        int32 Level,
        FString* OutFailureReason = nullptr) const;

	UItemWidget* CreateItemWidget(UWorld* WorldContext, const FInventorySlot& Item);
    UItemWidget* CreateItemWidget(UUserWidget* WidgetContext, const FInventorySlot& Item);
    UItemWidget* CreateItemWidgetEmpty();

    TArray<FItemDropInfo> GetRandomDropItem(const FName& MonsterID) const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    const FString& RunOptionRollStressTest(FName ItemID, int32 Count = 1000, int32 Level = 1);

    const FString& GetLastOptionRollStressReport() const { return LastOptionRollStressReport; }
private:
    void LoadItemData();
    void LoadOptionData();
    void LoadDropData();

private:
    UPROPERTY()
    UDataTable* ItemDataTable;
    
    UPROPERTY()
    FString ItemDataTablePath = TEXT("/Game/Datatable/DT_DiaitemTable.DT_DiaitemTable");
    
    UPROPERTY()
	UDataTable* OptionDataTable;
    
    UPROPERTY()
    FString OptionDataTablePath = TEXT("/Game/Datatable/DT_ItemOptions.DT_ItemOptions");

    UPROPERTY()
    FString DropDataTablePath = TEXT("/Game/Datatable/DT_MonsterDropTable.DT_MonsterDropTable");

    UPROPERTY()
    mutable TMap<FName, FItemBase> ItemCache;
    
    UPROPERTY()
    mutable TMap<FName, FDiaItemOptionRow> OptionCache;

    UPROPERTY()
	mutable TMap<FName, FMonsterDropTable> DropCache;

    UPROPERTY()
    FString LastOptionRollStressReport;

	void GenerateItemOptions(FItemInstance& Item, int32 Level);
    const FDiaItemOptionRow* FindOptionRowByKey(const FName& OptionKey) const;
    bool CanApplyOptionToItem(
        const FItemInstance& Item,
        const FDiaItemOptionRow& OptionRow,
        EItemOptionType ExpectedOptionType,
        int32 Level,
        FString* OutFailureReason = nullptr) const;
};
