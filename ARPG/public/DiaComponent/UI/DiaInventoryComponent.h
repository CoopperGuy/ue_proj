// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ItemBase.h"

#include "DiaInventoryComponent.generated.h"

class UMainInventory;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UDiaInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    bool TryAddItem(const FInventorySlot& ItemData, UMainInventory* InvenWidget);
	void FillGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);

    bool RemoveItem(const FGuid& InstanceID, UMainInventory* InventoryWidget);
    bool MoveItem(const FGuid& InstanceID, int32 NewPosX, int32 NewPosY);

	// 그리드 크기 가져오기
	FORCEINLINE int32 GetGridWidth() const { return InventoryGrid.Width; }
	FORCEINLINE int32 GetGridHeight() const { return InventoryGrid.Height; }
	FORCEINLINE const FGrid& GetInventoryGrid() const { return InventoryGrid; }
	
	// 인벤토리 검증 함수들
	bool CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY) const;
	bool FindPlaceForItem(int32 ItemWidth, int32 ItemHeight, int32& OutPosX, int32& OutPosY) const;

private:
    UPROPERTY()
    TArray<FInventorySlot> Items;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridWidth = 10;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridHeight = 5;
    
    UPROPERTY()
    FGrid InventoryGrid;
};
