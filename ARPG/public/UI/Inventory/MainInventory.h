// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "MainInventory.generated.h"

class UCanvasPanelSlot;
class UCanvasPanel;
class UItemWidget;
/**
 * 
 */
UCLASS()
class ARPG_API UMainInventory : public UUserWidget
{
	GENERATED_BODY()
	
public:	
	virtual void NativeConstruct() override;
	
	// Add item to inventory
	bool AddItemToInventory(const FInventoryItem& ItemData, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);
	//슬롯이 비어있는지 판단한다
	//해당 슬롯에 들어잇는 아이템의 row colum을 판단해야한다.
	bool IsSlotEmpty(int32 SlotIndex) const;
protected:
	void CreateInventory();
	void ConfigInventorySlot(int32 SlotIndex, UCanvasPanelSlot* CanvasSlot);	

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 szSlot= 42;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* InventoryCanvas;

	UPROPERTY()
	TArray<UCanvasPanelSlot*> InventorySlots;
	
public:
	FORCEINLINE int32 GetInventorySize() const { return InventorySlots.Num(); }
	FORCEINLINE UCanvasPanel* GetInventoryPanel() const { return InventoryCanvas; }	
	FORCEINLINE UItemWidget* GetItemWidgetAt(int32 Index) const;
	void GetAllItemWidgets(TArray<UItemWidget*>& OutItemWidgets) const;
};
