// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/EquipSlot.h"
#include "UI/Inventory/EquipWidget.h"
#include "UI/Inventory/MainInventory.h"
#include "UI/Item/ItemWidget.h"
#include "UI/DragDrop/ItemDragDropOperation.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"


#include "DiaBaseCharacter.h"

#include "System/GameViewPort/DiaCustomGameViewPort.h"

#include "Utils/InventoryUtils.h"
#include "Logging/ARPGLogChannels.h"

void UEquipSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UEquipSlot::SetItemWidget(const FInventorySlot& InItemData)
{
	SlotItemWidget->SetItemInfo(InItemData);
	SlotItemWidget->SetItemDragDropState(static_cast<int32>(EItemDragDropType::EIDT_Equipment));
	SlotItemWidget->SetVisibility(ESlateVisibility::Visible);
}

void UEquipSlot::ClearItemWidget()
{
	SlotItemWidget->ClearItemInfo();
	SlotItemWidget->SetVisibility(ESlateVisibility::Collapsed);
}

//아이템 드래그 앤 드롭 되면 실행된다.
bool UEquipSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!IsValid(ItemDragOp))
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Invalid drag operation dropped on equip slot."));
		return false;
	}

	//같은 슬롯 타입이면 가능하다. (장비인지도 판별 가능)
	EEquipmentSlot EquipSlot = ItemDragOp->ItemData.ItemInstance.BaseItem.EquipmentSlot;
	if (EquipSlot != SlotType)
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Dropped item does not match equip slot type. Expected: %s, Got: %s"), *UEnum::GetValueAsString(SlotType), *UEnum::GetValueAsString(EquipSlot));
		return false;
	}

	//인벤토리에서 온 경우에만 가능.
	if (ItemDragOp->DragType == EItemDragDropType::EIDT_Inventory)
	{
		UMainInventory* SourceInventoryWidget = ItemDragOp->SourceInventoryWidget;
		if (!IsValid(SourceInventoryWidget) && IsValid(ItemDragOp->SourceWidget))
		{
			SourceInventoryWidget = ItemDragOp->SourceWidget->GetTypedOuter<UMainInventory>();
		}
		if (!IsValid(SourceInventoryWidget))
		{
			UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Source inventory widget not found."));
			if (IsValid(ItemDragOp->SourceWidget))
			{
				ItemDragOp->SourceWidget->SetRenderOpacity(1.0f);
			}
			return false;
		}

		UE_LOG(LogARPG, Log, TEXT("EquipSlot: Attempting to equip item from inventory. Item: %s"), *ItemDragOp->ItemData.ItemInstance.InstanceID.ToString());
		return AddItemFromInventory(ItemDragOp->ItemData, ItemDragOp->SourceWidget, SourceInventoryWidget);
	}

	UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Unsupported drag type dropped on equip slot. Expected Inventory, Got: %d"), static_cast<int32>(ItemDragOp->DragType));
	return false;
}

void UEquipSlot::HandleItemEquipped(const FEquippedItem& Item)
{
	OnItemEquipped.Broadcast(Item, SlotType);
}

bool UEquipSlot::AddItem(const FInventorySlot& ItemInstance, UItemWidget* ItemWidget, int32 PosY, int32 PosX)
{
	if(!IsValid(ItemWidget))
	{
		return false;
	}

	SetItemWidget(ItemInstance);

	//장착한다는 deleagte broadcast 실행
	HandleItemEquipped(FEquippedItem::FromInventorySlot(ItemInstance));

	//인벤토리에서 제거해야한다
	//인벤 델리게이트 실행하자.
	if (InventoryComponent.IsValid())
	{
		InventoryComponent.Get()->OnItemRemoved.Broadcast(ItemInstance.ItemInstance.InstanceID);
		UE_LOG(LogARPG, Log, TEXT("EquipSlot: Item removed from inventory after equipping. InstanceID: %s"), *ItemInstance.ItemInstance.InstanceID.ToString());
	}	
	return true;
}

bool UEquipSlot::AddItemFromInventory(const FInventorySlot& ItemInstance, UItemWidget* ItemWidget, UMainInventory* SourceInventoryWidget)
{
	if (!IsValid(ItemWidget) || !IsValid(SourceInventoryWidget))
	{
		return false;
	}

	if (!InventoryComponent.IsValid())
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: InventoryComponent is not valid."));
		ItemWidget->SetRenderOpacity(1.0f);
		return false;
	}

	UItemWidget* CurrentItemWidget = GetItemWidget();
	const bool bHasCurrentItem = IsValid(CurrentItemWidget) && !CurrentItemWidget->GetItemInfo().IsEmpty();
	if (!bHasCurrentItem)
	{
		return AddItem(ItemInstance, ItemWidget);
	}

	const FInventorySlot PreviousItem = CurrentItemWidget->GetItemInfo();

	// 새 아이템의 인벤토리 공간을 먼저 비워야 기존 장비가 되돌아갈 수 있다.
	if (!InventoryComponent->RemoveItem(ItemInstance.ItemInstance.InstanceID, SourceInventoryWidget))
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Failed to remove new item from inventory. InstanceID: %s"),
			*ItemInstance.ItemInstance.InstanceID.ToString());
		ItemWidget->SetRenderOpacity(1.0f);
		return false;
	}

	if (!InventoryComponent->TryAddItem(PreviousItem, SourceInventoryWidget))
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Failed to return previous item to inventory. Rollback new item. PreviousID: %s"),
			*PreviousItem.ItemInstance.InstanceID.ToString());
		InventoryComponent->TryAddItem(ItemInstance, SourceInventoryWidget);
		return false;
	}

	if (EquippementComponent.IsValid())
	{
		EquippementComponent->UnEquipItem(SlotType);
	}
	else
	{
		ClearItemWidget();
	}

	SetItemWidget(ItemInstance);
	HandleItemEquipped(FEquippedItem::FromInventorySlot(ItemInstance));

	UE_LOG(LogARPG, Log, TEXT("EquipSlot: Replaced equipped item. NewID: %s, PreviousID: %s"),
		*ItemInstance.ItemInstance.InstanceID.ToString(),
		*PreviousItem.ItemInstance.InstanceID.ToString());

	return true;
}

bool UEquipSlot::RemoveContainItem(const FGuid& ItemInstanceID)
{
	return false;
}

void UEquipSlot::UnEquipItem()
{
	UE_LOG(LogARPG, Log, TEXT("Unequipping item frwarom slot: %s"), *UEnum::GetValueAsString(SlotType));
	ClearItemWidget();
}

void UEquipSlot::SetInventoryComponent(UDiaInventoryComponent* InComponent)
{
	InventoryComponent = InComponent;
}

void UEquipSlot::SetEquipmentComponent(UDiaEquipmentComponent* InComponent)
{
	EquippementComponent = InComponent;
}
