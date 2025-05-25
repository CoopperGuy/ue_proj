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
    
    bool TryAddItem(const FInventoryItem& ItemData, UMainInventory* InvenWidget);
    bool FindPlaceItem(const FInventoryItem& ItemData, int32& OutPosX, int32& OutPosY);
    bool CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);	

	void FillGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);

    bool RemoveItem(const FGuid& InstanceID, UMainInventory* InventoryWidget);
    bool MoveItem(const FGuid& InstanceID, int32 NewPosX, int32 NewPosY);
private:
    UPROPERTY()
    TArray<FInventoryItem> Items;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridWidth = 10;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridHeight = 5;
    
    UPROPERTY()
    FGrid InventoryGrid;
};
