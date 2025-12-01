// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/EquipSlot.h"
#include "UI/Inventory/EquipWidget.h"
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
		return false;
	}

	//같은 슬롯 타입이면 가능하다. (장비인지도 판별 가능)
	EEquipmentSlot EquipSlot = ItemDragOp->ItemData.ItemInstance.BaseItem.EquipmentSlot;
	if (EquipSlot != SlotType)
	{
		return false;
	}

	//인벤토리에서 온 경우에만 가능.
	if (ItemDragOp->DragType == EItemDragDropType::EIDT_Inventory)
	{
		return AddItem(ItemDragOp->ItemData, ItemDragOp->SourceWidget);
	}

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
		UE_LOG(LogTemp, Log, TEXT("EquipSlot: Item removed from inventory after equipping. InstanceID: %s"), *ItemInstance.ItemInstance.InstanceID.ToString());
	}	
	return true;
}

bool UEquipSlot::RemoveContainItem(const FGuid& ItemInstanceID)
{
	return false;
}

void UEquipSlot::UnEquipItem()
{
	UE_LOG(LogTemp, Log, TEXT("Unequipping item frwarom slot: %s"), *UEnum::GetValueAsString(SlotType));
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