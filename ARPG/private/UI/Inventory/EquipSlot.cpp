// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/EquipSlot.h"
#include "UI/Inventory/EquipWidget.h"
#include "UI/Inventory/MainInventory.h"
#include "UI/Item/ItemWidget.h"
#include "UI/DragDrop/ItemDragDropOperation.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/ContentWidget.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"


#include "DiaBaseCharacter.h"

#include "System/GameViewPort/DiaCustomGameViewPort.h"
#include "System/ItemSubsystem.h"

#include "Utils/InventoryUtils.h"
#include "Logging/ARPGLogChannels.h"

void UEquipSlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (!IsValid(itemSlot))
	{
		itemSlot = Cast<USizeBox>(GetWidgetFromName(TEXT("ItemSlot")));
	}
	if (!IsValid(itemSlot))
	{
		itemSlot = Cast<USizeBox>(GetWidgetFromName(TEXT("ItemSlotBox")));
	}
	if (IsValid(Image_Hover))
	{
		Image_Hover->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEquipSlot::SetItemWidget(const FInventorySlot& InItemData)
{
	bool bItemInfoAlreadySet = false;
	UItemWidget* NewItemWidget = nullptr;
	if (IsValid(itemSlot))
	{
		NewItemWidget = FInventoryUtils::CreateItemWidget(this, &InItemData);
		if (IsValid(NewItemWidget))
		{
			itemSlot->SetContent(NewItemWidget);
			SlotItemWidget = NewItemWidget;
			bItemInfoAlreadySet = true;
		}
	}

	if (!IsValid(SlotItemWidget))
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: SlotItemWidget is null. SlotType: %s"), *UEnum::GetValueAsString(SlotType));
		return;
	}

	if (IsValid(itemSlot))
	{
		itemSlot->SetVisibility(ESlateVisibility::Visible);
		itemSlot->SetRenderOpacity(1.0f);
		itemSlot->SetWidthOverride(100.0f);
		itemSlot->SetHeightOverride(100.0f);
		itemSlot->SetMinDesiredWidth(100.0f);
		itemSlot->SetMinDesiredHeight(100.0f);
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(itemSlot->Slot))
		{
			const FVector2D CurrentSize = CanvasSlot->GetSize();
			if (CurrentSize.X <= 0.0f || CurrentSize.Y <= 0.0f)
			{
				CanvasSlot->SetSize(FVector2D(100.0f, 100.0f));
			}
			CanvasSlot->SetZOrder(100);
		}
	}

	SlotItemWidget->SetVisibility(ESlateVisibility::Visible);
	SlotItemWidget->SetRenderOpacity(1.0f);
	if (!bItemInfoAlreadySet)
	{
		SlotItemWidget->SetItemInfo(InItemData);
	}
	SlotItemWidget->SetItemDragDropState(static_cast<int32>(EItemDragDropType::EIDT_Equipment));
	SlotItemWidget->SetVisibility(ESlateVisibility::Visible);
	SlotItemWidget->InvalidateLayoutAndVolatility();
	InvalidateLayoutAndVolatility();
	if (IsValid(Image_Frame))
	{
		Image_Frame->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (IsValid(Image_Hover))
	{
		Image_Hover->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UEquipSlot::ClearItemWidget()
{
	if (!IsValid(SlotItemWidget))
	{
		return;
	}

	SlotItemWidget->ClearItemInfo();
	SlotItemWidget->SetVisibility(ESlateVisibility::Collapsed);
	if (IsValid(Image_Frame))
	{
		Image_Frame->SetVisibility(ESlateVisibility::Visible);
	}
	if (IsValid(Image_Hover))
	{
		Image_Hover->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool UEquipSlot::IsEmpty() const
{
	return !IsValid(SlotItemWidget) || SlotItemWidget->GetItemInfo().IsEmpty();
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
	UItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UItemSubsystem>();
	if (!ItemSubsystem)
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: ItemSubsystem is null."));
		return false;
	}

	EEquipmentSlot EquipSlot = ItemSubsystem->GetEquipmentSlot(ItemDragOp->ItemData.ItemInstance);
	if (!CheckEquipSlot(EquipSlot))
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
	}	
	return true;
}

bool UEquipSlot::AddItemFromInventory(const FInventorySlot& ItemInstance, UItemWidget* ItemWidget, UMainInventory* SourceInventoryWidget)
{
	auto AddEquipDebugEvent = [this](const FString& Message, bool bPassed)
	{
		if (EquippementComponent.IsValid())
		{
			EquippementComponent->AddDebugEvent(Message, bPassed);
		}
	};

	if (!IsValid(ItemWidget) || !IsValid(SourceInventoryWidget))
	{
		AddEquipDebugEvent(TEXT("Equip failed: invalid item widget or source inventory."), false);
		return false;
	}

	if (!InventoryComponent.IsValid())
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: InventoryComponent is not valid."));
		AddEquipDebugEvent(TEXT("Equip failed: inventory component is invalid."), false);
		ItemWidget->SetRenderOpacity(1.0f);
		return false;
	}

	UItemWidget* CurrentItemWidget = GetItemWidget();
	const bool bHasCurrentItem = IsValid(CurrentItemWidget) && !CurrentItemWidget->GetItemInfo().IsEmpty();
	AddEquipDebugEvent(FString::Printf(TEXT("Equip request: %s -> %s%s"),
		*ItemInstance.ItemInstance.ItemID.ToString(),
		*UEnum::GetValueAsString(SlotType),
		bHasCurrentItem ? TEXT(" with replacement") : TEXT("")), true);

	if (!bHasCurrentItem)
	{
		if (!InventoryComponent->RemoveItem(ItemInstance.ItemInstance.InstanceID, SourceInventoryWidget))
		{
			UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Failed to remove item from inventory before equipping. InstanceID: %s"),
				*ItemInstance.ItemInstance.InstanceID.ToString());
			AddEquipDebugEvent(FString::Printf(TEXT("Inventory remove failed: %s"),
				*ItemInstance.ItemInstance.ItemID.ToString()), false);
			ItemWidget->SetRenderOpacity(1.0f);
			return false;
		}

		AddEquipDebugEvent(FString::Printf(TEXT("Inventory remove passed: %s"),
			*ItemInstance.ItemInstance.ItemID.ToString()), true);
		SetItemWidget(ItemInstance);
		HandleItemEquipped(FEquippedItem::FromInventorySlot(ItemInstance));
		return true;
	}

	const FInventorySlot PreviousItem = CurrentItemWidget->GetItemInfo();

	// 새 아이템의 인벤토리 공간을 먼저 비워야 기존 장비가 되돌아갈 수 있다.
	if (!InventoryComponent->RemoveItem(ItemInstance.ItemInstance.InstanceID, SourceInventoryWidget))
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Failed to remove new item from inventory. InstanceID: %s"),
			*ItemInstance.ItemInstance.InstanceID.ToString());
		AddEquipDebugEvent(FString::Printf(TEXT("Inventory remove failed: %s"),
			*ItemInstance.ItemInstance.ItemID.ToString()), false);
		ItemWidget->SetRenderOpacity(1.0f);
		return false;
	}

	AddEquipDebugEvent(FString::Printf(TEXT("Inventory remove passed: %s"),
		*ItemInstance.ItemInstance.ItemID.ToString()), true);
	if (!InventoryComponent->TryAddItem(PreviousItem, SourceInventoryWidget))
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Failed to return previous item to inventory. Rollback new item. PreviousID: %s"),
			*PreviousItem.ItemInstance.InstanceID.ToString());
		AddEquipDebugEvent(FString::Printf(TEXT("Previous return failed: %s"),
			*PreviousItem.ItemInstance.ItemID.ToString()), false);
		InventoryComponent->TryAddItem(ItemInstance, SourceInventoryWidget);
		AddEquipDebugEvent(FString::Printf(TEXT("Rollback attempted: %s"),
			*ItemInstance.ItemInstance.ItemID.ToString()), false);
		return false;
	}

	AddEquipDebugEvent(FString::Printf(TEXT("Previous return passed: %s"),
		*PreviousItem.ItemInstance.ItemID.ToString()), true);
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
	AddEquipDebugEvent(FString::Printf(TEXT("Replacement equip finished: %s"),
		*ItemInstance.ItemInstance.ItemID.ToString()), true);

	return true;
}

bool UEquipSlot::RemoveContainItem(const FGuid& ItemInstanceID)
{
	return false;
}

void UEquipSlot::UnEquipItem()
{
	ClearItemWidget();
}

bool UEquipSlot::CheckEquipSlot(EEquipmentSlot InEquipSlot)
{
	if(InEquipSlot == SlotType)
	{
		return true;
	}

	if(SlotType == EEquipmentSlot::EES_RingL || SlotType == EEquipmentSlot::EES_RingR)
	{
		if (InEquipSlot == EEquipmentSlot::EES_RingL || InEquipSlot == EEquipmentSlot::EES_RingR)
		{
			return true;
		}
	}
	
	return false;
}

void UEquipSlot::SetInventoryComponent(UDiaInventoryComponent* InComponent)
{
	InventoryComponent = InComponent;
}

void UEquipSlot::SetEquipmentComponent(UDiaEquipmentComponent* InComponent)
{
	EquippementComponent = InComponent;
}
