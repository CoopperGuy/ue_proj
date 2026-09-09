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

	return HandleInventoryDrop(ItemDragOp->ItemData, ItemDragOp->DragType,
		ItemDragOp->SourceWidget, ItemDragOp->SourceInventoryWidget);
}

bool UEquipSlot::HandleInventoryDrop(const FInventorySlot& ItemData, EItemDragDropType DragType,
	UItemWidget* SourceWidget, UMainInventory* SourceInventoryWidget)
{
	//같은 슬롯 타입이면 가능하다. (장비인지도 판별 가능)
	UItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UItemSubsystem>();
	if (!ItemSubsystem)
	{
		return false;
	}

	EEquipmentSlot EquipSlot = ItemSubsystem->GetEquipmentSlot(ItemData.ItemInstance);
	if (!CheckEquipSlot(EquipSlot))
	{
		UE_LOG(LogARPG, Warning, TEXT("EquipSlot: Dropped item does not match equip slot type. Expected: %s, Got: %s"), *UEnum::GetValueAsString(SlotType), *UEnum::GetValueAsString(EquipSlot));
		return false;
	}

	//인벤토리에서 온 경우에만 가능.
	if (DragType == EItemDragDropType::EIDT_Inventory)
	{
		if (!IsValid(SourceInventoryWidget) && IsValid(SourceWidget))
		{
			SourceInventoryWidget = SourceWidget->GetTypedOuter<UMainInventory>();
		}
		if (!IsValid(SourceInventoryWidget))
		{
			if (IsValid(SourceWidget))
			{
				SourceWidget->SetRenderOpacity(1.0f);
			}
			return false;
		}

		return AddItemFromInventory(ItemData, SourceWidget, SourceInventoryWidget);
	}

	return false;
}

void UEquipSlot::HandleItemEquipped(const FEquippedItem& Item)
{
	OnItemEquipped.Broadcast(Item, SlotType);
}

bool UEquipSlot::AddItem(const FInventorySlot& ItemInstance, UItemWidget* ItemWidget, int32 PosY, int32 PosX)
{
	return false;
}

bool UEquipSlot::AddItemFromInventory(const FInventorySlot& ItemInstance, UItemWidget* ItemWidget, UMainInventory* SourceInventoryWidget)
{

	if (!InventoryComponent.IsValid() || !EquippementComponent.IsValid())
		return false;

	UItemWidget* CurrentItemWidget = GetItemWidget();
	const bool bHasCurrentItem = EquippementComponent->GetEquippedItem(SlotType) == nullptr ? false : true;
	const FInventorySlot ItemInfo = ItemInstance;

	if (bHasCurrentItem)
	{
		// 새 아이템의 인벤토리 공간을 먼저 비워야 기존 장비가 되돌아갈 수 있다.
		FInventorySlot PrevItem;
		if (!EquippementComponent->TryUnEquipToInventory(SlotType, PrevItem))
		{
			return false;
		}
	}

	if (!EquippementComponent->TryEquipFromInventory(ItemInfo, SlotType))
		return false;

	SetItemWidget(ItemInfo);

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
