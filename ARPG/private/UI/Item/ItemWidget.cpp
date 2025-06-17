// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/ItemWidget.h"
#include "UI/DragDrop/ItemDragDropOperation.h"
#include "UI/Inventory/MainInventory.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UItemWidget::SetItemInfo(const FInventoryItem& ItemData)
{
	ItemInfo = ItemData;
	if (ItemIcon)
	{
		if (ItemInfo.IconPath.IsValid())
		{
			// FSoftObjectPath를 통해 텍스처 비동기 로딩
			TSoftObjectPtr<UTexture2D> IconTexture(ItemInfo.IconPath);
			if (UTexture2D* Icon = IconTexture.LoadSynchronous())
			{
				ItemIcon->SetBrushFromTexture(Icon);
			}
		}
	}
}

void UItemWidget::SetWidgetGridPos(int32 PositionX, int32 PositionY)
{
	ItemInfo.GridX = PositionX;
	ItemInfo.GridY = PositionY;
}

FReply UItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 왼쪽 마우스 버튼으로 드래그 감지 시작
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	
	return FReply::Unhandled();
}

void UItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	// 드래그 오퍼레이션 생성
	UItemDragDropOperation* DragOperation = NewObject<UItemDragDropOperation>();
	
	// 드래그 정보 설정
	DragOperation->ItemData = ItemInfo;
	DragOperation->SourceWidget = this;
	DragOperation->ItemWidth = ItemInfo.Width;
	DragOperation->ItemHeight = ItemInfo.Height;
	
	// 드래그 시각적 위젯 생성 (원본의 복사본)
	UItemWidget* DragVisual = CreateWidget<UItemWidget>(GetWorld(), GetClass());
	if (IsValid(DragVisual))
	{
		DragVisual->SetItemInfo(ItemInfo);
		DragVisual->SetRenderOpacity(0.5f); // 반투명하게 표시
		
		// 드래그 시각적 설정
		DragOperation->DefaultDragVisual = DragVisual;
		DragOperation->DragOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		DragOperation->Pivot = EDragPivot::MouseDown;
	}
	
	// 원본 위젯은 드래그 중임을 표시
	SetRenderOpacity(0.5f);
	
	OutOperation = DragOperation;
}

bool UItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 아이템 드래그 오퍼레이션인지 확인
	UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!ItemDragOp)
		return false;
	
	// 자기 자신에게 드롭하는 경우 처리하지 않음
	if (ItemDragOp->SourceWidget == this)
		return false;
	
	// 부모 인벤토리 위젯 찾기
	UMainInventory* ParentInventory = Cast<UMainInventory>(GetParent());
	if (!ParentInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("Parent inventory not found"));
		return false;
	}

	// 소스 아이템과 타겟 아이템 정보
	UItemWidget* SourceWidget = Cast<UItemWidget>(ItemDragOp->SourceWidget);
	if (!SourceWidget)
	{
		return false;
	}

	//FInventoryItem SourceItem = ItemDragOp->ItemData;
	//FInventoryItem TargetItem = ItemInfo;

	//// 아이템 교환 요청
	//bool bSuccess = ParentInventory->SwapItems(SourceItem, TargetItem);
	//
	//// 드래그 중인 위젯의 투명도 복구
	//if (SourceWidget)
	//{
	//	SourceWidget->SetRenderOpacity(1.0f);
	//}
	
	return bSuccess;
}

void UItemWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	
	// 드래그 취소 시 원본 위젯 복원
	SetRenderOpacity(1.0f);
}
