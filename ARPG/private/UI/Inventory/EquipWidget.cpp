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
#include "Logging/ARPGLogChannels.h"

void UEquipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	auto ResolveSlot = [this](UEquipSlot*& _Slot, const TArray<FName>& CandidateNames)
	{
		if (IsValid(_Slot))
		{
			return;
		}

		for (const FName& CandidateName : CandidateNames)
		{
			if (UEquipSlot* FoundSlot = Cast<UEquipSlot>(GetWidgetFromName(CandidateName)))
			{
				_Slot = FoundSlot;
				return;
			}
		}
	};

	ResolveSlot(BeltSlot, { TEXT("BeltSlot"), TEXT("PantsSlot"), TEXT("LegsSlot"), TEXT("LowerSlot") });
	ResolveSlot(ShieldSlot, { TEXT("ShieldSlot"), TEXT("RightWeaponSlot"), TEXT("OffHandSlot") });
	ResolveSlot(RingLSlot, { TEXT("RingLSlot"), TEXT("LeftRingSlot"), TEXT("RingLeftSlot"), TEXT("RingSlot2") });
	ResolveSlot(RingRSlot, { TEXT("RingRSlot"), TEXT("RightRingSlot"), TEXT("RingRightSlot"), TEXT("RingSlot") });
	ResolveSlot(AmuletSlot, { TEXT("AmuletSlot"), TEXT("NecklaceSlot"), TEXT("NeckSlot") });

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
	if (BeltSlot)
	{
		BeltSlot->SetSlotType(EEquipmentSlot::EES_Belt);
	}
	if (ShieldSlot)
	{
		ShieldSlot->SetSlotType(EEquipmentSlot::EES_Shield);
	}
	if (RingLSlot)
	{
		RingLSlot->SetSlotType(EEquipmentSlot::EES_RingL);
	}
	if (RingRSlot)
	{
		RingRSlot->SetSlotType(EEquipmentSlot::EES_RingR);
	}
	if (AmuletSlot)
	{
		AmuletSlot->SetSlotType(EEquipmentSlot::EES_Amulet);
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
		UE_LOG(LogARPG, Warning, TEXT("Invalid Slot Type: %s"), *UEnum::GetValueAsString(SlotType));
		return;
	}

	//이미 아이템이 장착되어 있다면, 종료
	//수동으로 갈아 껴야한다.
	if(TargetSlot->IsEmpty() == false)
	{
		return;
	}

	TargetSlot->SetItemWidget(Item.ToInventorySlot());
}

void UEquipWidget::UnequipItemFromSlot(EEquipmentSlot SlotType)
{
	if (SlotType == EEquipmentSlot::EES_None)
	{
		UE_LOG(LogARPG, Warning, TEXT("Invalid Slot Type: %s"), *UEnum::GetValueAsString(SlotType));
		return ;
	}
	UEquipSlot* TargetSlot = GetEquipSlot(SlotType);
	if (IsValid(TargetSlot))
	{
		UE_LOG(LogARPG, Warning, TEXT("Invalid Slot Type: %s"), *UEnum::GetValueAsString(SlotType));
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

