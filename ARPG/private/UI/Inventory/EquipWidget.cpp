// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/EquipWidget.h"
#include "UI/Inventory/EquipSlot.h"
#include "UI/Item/ItemWidget.h"
#include "Utils/InventoryUtils.h"

#include "Controller/DiaController.h"

#include "Components/Image.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"

#include "UI/DragDrop/ItemDragDropOperation.h"

void UEquipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ArmorSlot)
	{
		ArmorSlot->SetSlotType(EEquipmentSlot::EES_Chest);
	}
	if (LeftWeaponSlot)
	{
		LeftWeaponSlot->SetSlotType(EEquipmentSlot::EES_Weapon);
	}
	if (ShooseSlot)
	{
		ShooseSlot->SetSlotType(EEquipmentSlot::EES_Feet);
	}
	if (HandsSlot)
	{
		HandsSlot->SetSlotType(EEquipmentSlot::EES_Hands);
	}
	if (HelmetSlot)
	{
		HelmetSlot->SetSlotType(EEquipmentSlot::EES_Head);
	}

}

void UEquipWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UEquipWidget::EquipItemToSlot(EEquipmentSlot SlotType, const FEquippedItem& Item)
{
	if (SlotType == EEquipmentSlot::EES_None)
	{
		return;
	}
	UEquipSlot* TargetSlot = GetEquipSlot(SlotType);
	if (!IsValid(TargetSlot))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Slot Type: %s"), *UEnum::GetValueAsString(SlotType));
		return;
	}

	//이미 아이템이 장착되어 있다면, 종료
	//수동으로 갈아 껴야한다.
	if(TargetSlot->IsEmpty() == false)
	{
		return;
	}

	//이미 아이템 위젯이 있다는 거기 때문에 반환
	UItemWidget* ItemWidget = TargetSlot->GetItemWidget();
	if (IsValid(ItemWidget))
	{
		return;
	}
	}

void UEquipWidget::UnequipItemFromSlot(EEquipmentSlot SlotType)
{
	if (SlotType == EEquipmentSlot::EES_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Slot Type: %s"), *UEnum::GetValueAsString(SlotType));
		return ;
	}
	UEquipSlot* TargetSlot = GetEquipSlot(SlotType);
	if (IsValid(TargetSlot))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Slot Type: %s"), *UEnum::GetValueAsString(SlotType));
	}
	TargetSlot->UnEquipItem();

	return ;
}

void UEquipWidget::SetInventoryComponent(UDiaInventoryComponent* InComponent)
{
	InventoryComponent = InComponent;

	for (int32 i = 0; i < static_cast<int32>(EEquipmentSlot::EES_Max); ++i)
	{
		EEquipmentSlot SlotType = static_cast<EEquipmentSlot>(i);
		UEquipSlot* SlotWidget = GetEquipSlot(SlotType);
		if (IsValid(SlotWidget))
		{
			SlotWidget->SetInventoryComponent(InventoryComponent.Get());
		}
	}
}

void UEquipWidget::SetEquipmentComponent(UDiaEquipmentComponent* InComponent)
{
	EquipementComponent = InComponent;

	for (int32 i = 0; i < static_cast<int32>(EEquipmentSlot::EES_Max); ++i)
	{
		EEquipmentSlot SlotType = static_cast<EEquipmentSlot>(i);
		UEquipSlot* SlotWidget = GetEquipSlot(SlotType);
		if (IsValid(SlotWidget))
		{
			SlotWidget->SetEquipmentComponent(EquipementComponent.Get());
		}
	}

	if (EquipementComponent->IsValidLowLevel())
	{
		EquipementComponent->OnItemUnEquipped.AddDynamic(DiaControllerRef.Get(), &ADiaController::OnUnequipItemProgress);
		EquipementComponent->OnItemUnEquipped.AddDynamic(this, &ThisClass::UnequipItemFromSlot);
	}

}

void UEquipWidget::SetDiaController(ADiaController* InController)
{
	DiaControllerRef = InController;
}

//EquipSlot이 false일 경우 실행된다.
bool UEquipWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	//드래그 시 오퍼레이션
	UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!IsValid(ItemDragOp))
	{
		return false;
	}


	return false;
}

