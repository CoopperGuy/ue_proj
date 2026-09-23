// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "Interface/ItemContainer.h"
#include "EquipSlot.generated.h"

class UImage;
class UItemWidget;
enum class EItemDragDropType : uint8;
class USizeBox;
class UMainInventory;
class UDiaInventoryComponent;
class UDiaEquipmentComponent;

/**
 * 
 */
UCLASS()
class ARPG_API UEquipSlot : public UUserWidget, public IItemContainer
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;
	void NativeDestruct() override;

	// ItemWidget 관리 함수들
	void SetItemWidget(const FInventorySlot& InItemData);
	void ClearItemWidget();

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION()
	void HandleItemEquipped(const FEquippedItem& Item, EEquipmentSlot EquippedSlot);

	virtual bool AddItem(const FInventorySlot& ItemInstance, UItemWidget* ItemWidget, int32 PosY = -1, int32 PosX = -1) override;
	bool RemoveContainItem(const FGuid& ItemInstanceID);

	UFUNCTION()
	void UnEquipItem(EEquipmentSlot UnequippedSlot);
protected:
	bool HandleInventoryDrop(const FInventorySlot& ItemData, EItemDragDropType DragType,
		UItemWidget* SourceWidget, UMainInventory* SourceInventoryWidget);

	UPROPERTY(meta = (BindWidget))
	UImage* Image_Frame;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Image_Hover;

	UPROPERTY(meta = (BindWidgetOptional))
	USizeBox* itemSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equip" , meta = (AllowPrivateAccess = "true"))
	EEquipmentSlot SlotType;

	UPROPERTY(meta = (BindWidget))
	UItemWidget* SlotItemWidget;

	TWeakObjectPtr<UMainInventory> MainInventoryWidget;
	TWeakObjectPtr<UDiaInventoryComponent> InventoryComponent;
	TWeakObjectPtr<UDiaEquipmentComponent> EquippementComponent;

public:
	void SetSlotType(EEquipmentSlot NewSlotType) { SlotType = NewSlotType; }
	FORCEINLINE EEquipmentSlot	GetSlotType() const { return SlotType; }
	bool IsEmpty() const;
	FORCEINLINE UItemWidget* GetItemWidget() const { return SlotItemWidget; }
	void SetInventoryComponent(UDiaInventoryComponent* InComponent);
	void SetMainInventoryWidget(UMainInventory* InWidget);
	void SetEquipmentComponent(UDiaEquipmentComponent* InComponent);
};
