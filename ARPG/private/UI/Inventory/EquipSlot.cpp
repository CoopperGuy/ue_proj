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
}

void UEquipSlot::ClearItemWidget()
{
	SlotItemWidget->ClearItemInfo();
}

void UEquipSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	if (!SlotItemWidget->HasItemInfo())
	{
		return;
	}
	if (!IsValid(SlotItemWidget))
	{
		return;
	}

	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	// 드래그 오퍼레이션 생성
	UItemDragDropOperation* DragOperation = NewObject<UItemDragDropOperation>();

	// 드래그 정보 설정
	DragOperation->ItemData = SlotItemWidget->GetItemInfo();
	DragOperation->SourceWidget = SlotItemWidget;
	DragOperation->ItemWidth = SlotItemWidget->GetItemInfo().ItemInstance.GetWidth();
	DragOperation->ItemHeight = SlotItemWidget->GetItemInfo().ItemInstance.GetHeight();

	// 드래그 시각적 위젯 생성 (원본의 복사본)
	UItemWidget* DragVisual = CreateWidget<UItemWidget>(GetWorld(), GetClass());
	if (IsValid(DragVisual))
	{
		DragVisual->SetItemInfo(SlotItemWidget->GetItemInfo());
		DragVisual->SetRenderOpacity(0.5f); // 반투명하게 표시

		// 드래그 시각적 설정
		DragOperation->DefaultDragVisual = DragVisual;
		DragOperation->DragOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		DragOperation->Pivot = EDragPivot::MouseDown;
	}

	// 원본 위젯은 드래그 중임을 표시
	SetRenderOpacity(0.5f);

	// 커스텀 뷰포트 클라이언트에 드래그 시작 알림
	if (GEngine && GEngine->GameViewport)
	{
		if (UDiaCustomGameViewPort* CustomViewport = Cast<UDiaCustomGameViewPort>(GEngine->GameViewport))
		{
			CustomViewport->OnDragStarted(DragOperation);
		}
	}

	OutOperation = DragOperation;
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

	return AddItem(ItemDragOp->ItemData, ItemDragOp->SourceWidget);
}

void UEquipSlot::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	// 드래그 취소 시 원본 위젯 복원
	SetRenderOpacity(1.0f);

	//// 커스텀 뷰포트 클라이언트에 드래그 종료 알림
	//if (GEngine && GEngine->GameViewport)
	//{
	//	if (UDiaCustomGameViewPort* CustomViewport = Cast<UDiaCustomGameViewPort>(GEngine->GameViewport))
	//	{
	//		CustomViewport->OnDragEnd_CreateCautionWidget();
	//	}
	//}

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

	//장착한다.
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


void UEquipSlot::SetInventoryComponent(UDiaInventoryComponent* InComponent)
{
	InventoryComponent = InComponent;
}

void UEquipSlot::SetEquipmentComponent(UDiaEquipmentComponent* InComponent)
{
	EquippementComponent = InComponent;
}