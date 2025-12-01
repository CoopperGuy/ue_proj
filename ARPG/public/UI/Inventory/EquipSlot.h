// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "Interface/ItemContainer.h"
#include "EquipSlot.generated.h"

class UImage;
class UItemWidget;
class USizeBox;
class UDiaInventoryComponent;
class UDiaEquipmentComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquipped, const FEquippedItem&, Item, EEquipmentSlot, SlotType);

/**
 * 
 */
UCLASS()
class ARPG_API UEquipSlot : public UUserWidget, public IItemContainer
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;

	// ItemWidget 관리 함수들
	void SetItemWidget(const FInventorySlot& InItemData);
	void ClearItemWidget();

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	void HandleItemEquipped(const FEquippedItem& Item);

	virtual bool AddItem(const FInventorySlot& ItemInstance, UItemWidget* ItemWidget, int32 PosY = -1, int32 PosX = -1) override;
	bool RemoveContainItem(const FGuid& ItemInstanceID);

	void UnEquipItem();
public:
	FOnItemEquipped OnItemEquipped;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* SlotFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equip" , meta = (AllowPrivateAccess = "true"))
	EEquipmentSlot SlotType;

	UPROPERTY(meta = (BindWidget))
	UItemWidget* SlotItemWidget;

	TWeakObjectPtr<UDiaInventoryComponent> InventoryComponent;
	TWeakObjectPtr<UDiaEquipmentComponent> EquippementComponent;

public:
	void SetSlotType(EEquipmentSlot NewSlotType) { SlotType = NewSlotType; }
	FORCEINLINE EEquipmentSlot	GetSlotType() const { return SlotType; }
	bool IsEmpty() const { return SlotType == EEquipmentSlot::EES_None; }
	FORCEINLINE UItemWidget* GetItemWidget() const { return SlotItemWidget; }
	void SetInventoryComponent(UDiaInventoryComponent* InComponent);
	void SetEquipmentComponent(UDiaEquipmentComponent* InComponent);
};
