// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/EquipSlot.h"
#include "UI/Inventory/EquipWidget.h"
#include "UI/Item/ItemWidget.h"
#include "UI/DragDrop/ItemDragDropOperation.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"

#include "DiaBaseCharacter.h"

#include "Utils/InventoryUtils.h"

void UEquipSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UEquipSlot::SetItemWidget(const FInventorySlot& InItemData)
{
	SlotItemWidget->SetItemInfo(InItemData);
	HandleItemEquipped(FEquippedItem::FromInventorySlot(InItemData));
}

void UEquipSlot::ClearItemWidget()
{
	SlotItemWidget->ClearItemInfo();
}

//아이템 드래그 앤 드롭 되면 실행된다.
bool UEquipSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!IsValid(ItemDragOp))
	{
		return false;
	}

	//같은 슬롯 타입이면 가능하다.
	EEquipmentSlot EquipSlot = ItemDragOp->ItemData.ItemInstance.BaseItem.EquipmentSlot;
	if (EquipSlot != SlotType)
	{
		return false;
	}

	SetItemWidget(ItemDragOp->ItemData);

	return true;
}

void UEquipSlot::HandleItemEquipped(const FEquippedItem& Item)
{
	OnItemEquipped.Broadcast(Item, SlotType);
}
