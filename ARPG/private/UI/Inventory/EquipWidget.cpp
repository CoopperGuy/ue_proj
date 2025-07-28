// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/EquipWidget.h"
#include "UI/Inventory/EquipSlot.h"
#include "UI/Item/ItemWidget.h"
#include "Utils/InventoryUtils.h"

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

FEquippedItem UEquipWidget::UnequipItemFromSlot(EEquipmentSlot SlotType)
{
	// TODO: 여기에 return 문을 삽입합니다.
	return FEquippedItem{};
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

