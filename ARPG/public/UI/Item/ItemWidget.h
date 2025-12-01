// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "ItemWidget.generated.h"

class UImage;
class UEquipSlot;
class USizeBox;
/**
 * 
 */
UCLASS()
class ARPG_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
	// 드래그 앤 드롭 지원
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	void SetItemInfo(const FInventorySlot& ItemData);
	FVector2D CalculateIconSize(const FInventorySlot& ItemData) const;
	void SetIconSize(const FVector2D& NewSize);
    void SetWidgetGridPos(int32 PositionX, int32 PositionY);
	void SetWidgetPosition(int32 PositionX, int32 PositionY);
	
	bool MoveGridPosition(int32 DeltaX, int32 DeltaY);

	void ClearItemInfo();
	void DestoryItemWidget();

	bool HasItemInfo() const { return !ItemInfo.IsEmpty(); }
	// Getter 함수들
	FORCEINLINE const FInventorySlot& GetItemInfo() const { return ItemInfo; }
	FORCEINLINE UEquipSlot* GetParentSlot() const { return ParentSlot; }
	FORCEINLINE const FGuid& GetItemInstanceID() const { return ItemInfo.ItemInstance.InstanceID; }
	//include 하기 싫어서 상수로 정의함.
	FORCEINLINE void SetItemDragDropState(int32 State) { ItemDragDropState = State; }
	FORCEINLINE int32 GetItemDragDropState() const { return ItemDragDropState; }
protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	USizeBox* ItemSzBox;

	UPROPERTY(BlueprintReadOnly)
	FInventorySlot ItemInfo;
	
	// 장비 슬롯 부모 위젯 참조
	UPROPERTY()
	UEquipSlot* ParentSlot;

	const float BaseSlotSize = 52.0f;
	
	// 0: None, 1: Dragging, 2: Dropped
	int32 ItemDragDropState = 0; 
private:
	bool ValidateIconComponents() const;
	void ConfigureSizeBox(const FVector2D& NewSize);
	void ConfigureCanvasSlot(UWidget* Widget, const FVector2D& NewSize);
	void ForceLayoutUpdate();
};
