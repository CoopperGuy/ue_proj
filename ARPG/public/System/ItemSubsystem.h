// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/ItemBase.h"  
#include "ItemSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    // USubsystem �������̽�
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // ������ ������ �ε�
    UFUNCTION(BlueprintCallable, Category = "Items")
    void LoadItemData();
    
    // ������ ������ ��ȸ
    UFUNCTION(BlueprintCallable, Category = "Items")
    const FItemBase& GetItemData(const FName& ItemID) const;
    
    // �ν��Ͻ� ������ ����
    UFUNCTION(BlueprintCallable, Category = "Items")
    FInventoryItem CreateItemInstance(const FName& ItemID, int32 Level = 1, bool bRandomStats = false);
    
    // Ư�� ������ ������ �˻�
    //UFUNCTION(BlueprintCallable, Category = "Items")
    //TArray<FName> GetItemsByFilter(const FItemFilter& Filter);
    
private:
    UPROPERTY()
    UDataTable* ItemDataTable;
    
    // ��� ����
    UPROPERTY()
    FString ItemDataTablePath = TEXT("/Game/Datatable/DT_DiaitemTable.DT_DiaitemTable");
    
    // ĳ�� �����̳�
    UPROPERTY()
    mutable TMap<FName, FItemBase> ItemCache;
    
    // ��ƺ��� ��Ÿ�� ���� �Ӽ� 
    void GenerateRandomStats(FInventoryItem& Item, int32 Level);
};
    