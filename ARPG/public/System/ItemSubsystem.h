// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/ItemBase.h"  
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
    
    UFUNCTION(BlueprintCallable, Category = "Items")
    void LoadItemData();
    
    UFUNCTION(BlueprintCallable, Category = "Items")
    const FItemBase& GetItemData(const FName& ItemID) const;
    
    UFUNCTION(BlueprintCallable, Category = "Items")
    FInventorySlot CreateItemInstance(const FName& ItemID, int32 Level = 1, bool bRandomStats = false);
    
	UItemWidget* CreateItemWidget(const FInventorySlot& Item);
    //UFUNCTION(BlueprintCallable, Category = "Items")
    //TArray<FName> GetItemsByFilter(const FItemFilter& Filter);
    
private:
    UPROPERTY()
    UDataTable* ItemDataTable;
    
    UPROPERTY()
    FString ItemDataTablePath = TEXT("/Game/Datatable/DT_DiaitemTable.DT_DiaitemTable");
    
    UPROPERTY()
    mutable TMap<FName, FItemBase> ItemCache;
    
    void GenerateRandomStats(FInventorySlot& Item, int32 Level);
};
    