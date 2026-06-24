// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ItemBase.h"

#include "DiaInventoryComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, const FGuid&, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoldChanged, int32, NewGoldAmount, int32, DeltaAmount);

class UMainInventory;
class UItemWidget;
class UDiaOptionManagerComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UDiaInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaInventoryComponent();

    UPROPERTY(BlueprintAssignable)
    FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable)
	FOnGoldChanged OnGoldChanged;
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void HandleItemRemoved(const FGuid& ItemID);

public:	    
    bool TryAddItem(const FInventorySlot& ItemData, UMainInventory* InvenWidget);
	bool RequestMoveItem(const FGuid& InstanceID, int32 DestX, int32 DestY, UMainInventory* InventoryWidget);
	bool RemoveItem(const FGuid& InstanceID, UMainInventory* InvenWidget);

	
	// 인벤토리 검증 함수들
	bool CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY) const;
	bool FindPlaceForItem(int32 ItemWidth, int32 ItemHeight, int32& OutPosX, int32& OutPosY) const;

	void AddGoldInventoryWithCheckOption(int32 Amount, UDiaOptionManagerComponent* OptionManager);

	void SaveInventoryToSaveGame(class UDiaSaveGame* SaveGameInstance) const;
	void LoadInventoryFromSaveGame(const class UDiaSaveGame* SaveGameInstance);
private:
	void FillGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY, const FItemInstance& InstanceInfo);
	bool ClearGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);
private:

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridWidth = 10;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridHeight = 5;
    
    UPROPERTY()
    FInventoryGrid InventoryGrid;

	UPROPERTY()
	int32 Gold = 0;

	UPROPERTY()
	TMap<FGuid, FInventorySlot> InventoryItems;
public:
	// 그리드 크기 가져오기
	FORCEINLINE int32 GetGridWidth() const { return InventoryGrid.Width; }
	FORCEINLINE int32 GetGridHeight() const { return InventoryGrid.Height; }
	FORCEINLINE const FInventoryGrid& GetInventoryGrid() const { return InventoryGrid; }
	FORCEINLINE const int32 GetGold() const { return Gold; }
	void SetGold(int32 NewGold);
	void AddGold(int32 Amount);
	const TMap<FGuid, FInventorySlot>& GetInventoryItems() const { return InventoryItems; }
	const FInventorySlot* GetItemDataAtGuid(const FGuid& InstanceID) const;
};
