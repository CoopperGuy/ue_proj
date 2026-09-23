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
	UItemWidget* OldItemWidget = SlotItemWidget;
	if (IsValid(itemSlot))
	{
		NewItemWidget = FInventoryUtils::CreateItemWidget(this, &InItemData);
		if (IsValid(NewItemWidget))
		{
			if(IsValid(OldItemWidget))
				OldItemWidget->SetParentSlot(nullptr);

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
	SlotItemWidget->SetParentSlot(this);

	InvalidateLayoutAndVolatility();
	if (IsValid(Image_Frame))
	{
		Image_Frame->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (IsValid(Image_Hover))
	{
		Image_Hover->SetVisibility(ESlateVisibility::Collapsed);
	}

	SlotItemWidget->OnItemRightClicked.BindLambda([this](UItemWidget* ClickedItemWidget)
	{
		if (!IsValid(ClickedItemWidget) || !EquippementComponent.Get())
		{
			return;
		}
		const FInventorySlot& ItemInfo = ClickedItemWidget->GetItemInfo();
		if (ItemInfo.IsEmpty())
		{
			return;
		}

		IItemContainer* TargetContainer = Cast<IItemContainer>(MainInventoryWidget.Get());
		if (TargetContainer)
		{
			TargetContainer->AddItem(ItemInfo, ClickedItemWidget);
		}
	});
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

		const bool bSuccess = AddItem(ItemData, SourceWidget);
		if (!bSuccess && IsValid(SourceWidget))
		{
			SourceWidget->SetRenderOpacity(1.0f);
		}
		return bSuccess;

	}

	return false;
}

void UEquipSlot::HandleItemEquipped(const FEquippedItem& Item, EEquipmentSlot EquippedSlot)
{
	if (EquippedSlot == SlotType)
	{
		SetItemWidget(Item.ToInventorySlot());
	}
}

bool UEquipSlot::AddItem(const FInventorySlot& ItemInstance, UItemWidget* ItemWidget, int32 PosY, int32 PosX)
{
	return EquippementComponent.IsValid() &&
		EquippementComponent->TryEquipFromInventory(ItemInstance, SlotType);
}

bool UEquipSlot::RemoveContainItem(const FGuid& ItemInstanceID)
{
	if (!IsValid(SlotItemWidget) || SlotItemWidget->GetItemInstanceID() != ItemInstanceID)
	{
		return false;
	}
	ClearItemWidget();
	return true;
}

void UEquipSlot::UnEquipItem(EEquipmentSlot UnequippedSlot)
{
	if (UnequippedSlot == SlotType)
	{
		ClearItemWidget();
	}
}

void UEquipSlot::SetInventoryComponent(UDiaInventoryComponent* InComponent)
{
	InventoryComponent = InComponent;
}

void UEquipSlot::SetMainInventoryWidget(UMainInventory* InWidget)
{
	MainInventoryWidget = InWidget;
}

void UEquipSlot::SetEquipmentComponent(UDiaEquipmentComponent* InComponent)
{
	if (EquippementComponent.IsValid())
	{
		EquippementComponent->OnItemEquipped.RemoveDynamic(this, &ThisClass::HandleItemEquipped);
		EquippementComponent->OnItemUnEquipped.RemoveDynamic(this, &ThisClass::UnEquipItem);
	}

	EquippementComponent = InComponent;
	if (EquippementComponent.IsValid())
	{
		EquippementComponent->OnItemEquipped.AddUniqueDynamic(this, &ThisClass::HandleItemEquipped);
		EquippementComponent->OnItemUnEquipped.AddUniqueDynamic(this, &ThisClass::UnEquipItem);
	}
}

void UEquipSlot::NativeDestruct()
{
	SetEquipmentComponent(nullptr);
	Super::NativeDestruct();
}
