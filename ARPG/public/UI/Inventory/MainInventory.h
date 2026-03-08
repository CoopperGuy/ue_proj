// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "Interface/ItemContainer.h"
#include "MainInventory.generated.h"

class UCanvasPanelSlot;
class UCanvasPanel;
class UItemWidget;
class UDiaInventoryComponent;
class UDiaEquipmentComponent;
class UItemDragDropOperation;
/**
 * 
 */
UCLASS()
class ARPG_API UMainInventory : public UUserWidget, public IItemContainer
{
	GENERATED_BODY()
	
public:	
	virtual void NativeConstruct() override;
	void InitializeInventory();

	// Add item to inventory
	bool AddItemToInventory(const FInventorySlot& ItemData, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);
	// Remove item from inventory
	bool RemoveItemFromInventory(int32 SlotIndex);
	//슬롯이 비어있는지 판단한다
	//해당 슬롯에 들어잇는 아이템의 row colum을 판단해야한다.
	bool IsSlotEmpty(int32 SlotIndex) const;
	
	// 위치 기반 아이템 검색
	UItemWidget* GetItemWidgetAtGridPosition(int32 GridX, int32 GridY) const;
	UItemWidget* GetItemWidgetAt(int32 SlotIndex) const;
	UItemWidget* GetItemWidgetAtGuid(const FGuid& ItemInstanceID) const;
	const FInventorySlot* GetItemDataAtGuid(const FGuid& ItemInstanceID) const;

	// 새로운 드래그 프록시 방식
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	bool CheckInventoryDrop(UItemDragDropOperation* ItemDragOp, int32 NewGridX, int32 NewGridY, FVector2D& ScreenPosition, bool& retFlag);
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	// 그리드 좌표를 슬롯 인덱스로 변환
	//int32 GetSlotIndexFromPosition(const FVector2D& Position) const;
	FVector2D GetGridPositionFromScreenPosition(const FVector2D& ScreenPosition) const;
	FVector2D GetCanvasLocalPositionFromScreenPosition(const FGeometry& MainWidgetGeometry, const FVector2D& ScreenPosition) const;
	
	// 아이템 위치 업데이트
	void UpdateItemPosition(UItemWidget* ItemWidget, int32 NewGridX, int32 NewGridY);

	virtual bool AddItem(const FInventorySlot& Item, UItemWidget* ItemWidget, int32 PosY = -1, int32 PosX = -1);
	virtual bool RemoveContainItem(const FGuid& ItemInstanceID) override;
	bool MoveContainItem(const FGuid& ItemInstanceID, int32 NewPosX, int32 NewPosY);
protected:
	void CreateInventory();
	void ConfigInventorySlot(int32 SlotIndex, UCanvasPanelSlot* CanvasSlot);	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 szSlot= 52;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* InventoryCanvas;

	UPROPERTY()
	TArray<UCanvasPanelSlot*> InventorySlots;
	
	UPROPERTY()
	TMap<FGuid, UItemWidget*> ItemWidgets;

	TWeakObjectPtr<UDiaInventoryComponent> InventoryComponent;
	TWeakObjectPtr<UDiaEquipmentComponent> EquippementComponent;

	int32 GridWidth = 0; // 그리드 너비
	int32 GridHeight = 0; // 그리드 높이

public:
	FORCEINLINE int32 GetInventorySize() const { return InventorySlots.Num(); }
	FORCEINLINE UCanvasPanel* GetInventoryPanel() const { return InventoryCanvas; }	
	void GetAllItemWidgets(TArray<UUserWidget*>& OutItemWidgets) const;
	void SetInventoryComponent(UDiaInventoryComponent* InComponent);
	void SetEquipmentComponent(UDiaEquipmentComponent* InComponent);
private:
	// 드롭된 위치가 인벤토리와 장착 위젯 모두의 바깥인지 체크
	bool IsDropOutsideAllWidgets(const FVector2D& ScreenPosition) const;
};
