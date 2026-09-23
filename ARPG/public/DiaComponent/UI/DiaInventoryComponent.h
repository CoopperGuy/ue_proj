// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ItemBase.h"

#include "DiaInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemAdded, const FInventorySlot&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemRemoved, const FGuid&, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInventoryItemMoved, const FGuid&, ItemID, int32, GridX, int32, GridY);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGoldChanged, int32, NewGoldAmount, int32, DeltaAmount);

class UDiaOptionManagerComponent;
class UDiaSaveGame;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UDiaInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDiaInventoryComponent();

	UPROPERTY(BlueprintAssignable)
	FOnInventoryItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable)
	FOnInventoryItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable)
	FOnInventoryItemMoved OnItemMoved;

	UPROPERTY(BlueprintAssignable)
	FOnGoldChanged OnGoldChanged;

	// Suppress notifications only while an equipment transfer is being committed or rolled back.
	bool TryAddItem(const FInventorySlot& ItemData, int32 PosX = -1, int32 PosY = -1, bool bNotify = true);
	bool RequestMoveItem(const FGuid& InstanceID, int32 DestX, int32 DestY);
	bool RemoveItem(const FGuid& InstanceID, bool bNotify = true);
	bool CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY) const;
	bool FindPlaceForItem(int32 ItemWidth, int32 ItemHeight, int32& OutPosX, int32& OutPosY) const;

	void AddGoldInventoryWithCheckOption(int32 Amount, UDiaOptionManagerComponent* OptionManager);
	void SaveInventoryToSaveGame(UDiaSaveGame* SaveGameInstance) const;
	void LoadInventoryFromSaveGame(const UDiaSaveGame* SaveGameInstance);

	FORCEINLINE int32 GetGridWidth() const { return InventoryGrid.Width; }
	FORCEINLINE int32 GetGridHeight() const { return InventoryGrid.Height; }
	FORCEINLINE const FInventoryGrid& GetInventoryGrid() const { return InventoryGrid; }
	FORCEINLINE int32 GetGold() const { return Gold; }
	void SetGold(int32 NewGold);
	void AddGold(int32 Amount);
	const TMap<FGuid, FInventorySlot>& GetInventoryItems() const { return InventoryItems; }
	const FInventorySlot* GetItemDataAtGuid(const FGuid& InstanceID) const;

protected:
	virtual void BeginPlay() override;

private:
	void FillGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY, const FItemInstance& InstanceInfo);
	bool ClearGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);

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
};
