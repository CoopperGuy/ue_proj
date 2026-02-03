// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Item/ItemWidget.h"
#include "UI/DragDrop/ItemDragDropOperation.h"
#include "UI/Inventory/MainInventory.h"
#include "UI/Inventory/EquipSlot.h"
#include "UI/Item/ItemToolTipWidget.h"
#include "Utils/InventoryUtils.h"

#include "System/GameViewPort/DiaCustomGameViewPort.h"

#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Engine/Engine.h"

void UItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

//툴팁 설정을 여기서 하도록 변경
void UItemWidget::SetItemInfo(const FInventorySlot& ItemData)
{
	ItemInfo = ItemData;
	if (ItemIcon)
	{
		if (ItemInfo.ItemInstance.GetIconPath().IsValid())
		{
			// FSoftObjectPath를 통해 텍스처 비동기 로딩
			TSoftObjectPtr<UTexture2D> IconTexture(ItemInfo.ItemInstance.GetIconPath());
			if (UTexture2D* Icon = IconTexture.LoadSynchronous())
			{
				ItemIcon->SetBrushFromTexture(Icon);
				FVector2D IconSize = CalculateIconSize(ItemData);
				SetIconSize(IconSize);
			}
		}
	}

	FSoftObjectPath TooltipWidgetPath(TEXT("/Game/UI/Inventory/WBP_ItemToolTip.WBP_ItemToolTip_C"));
	TSoftClassPtr<UUserWidget> TooltipWidgetAssetPtr(TooltipWidgetPath);
	UClass* TooltipWidgetClass = TooltipWidgetAssetPtr.LoadSynchronous();

	if (TooltipWidgetClass)
	{
		UItemToolTipWidget* ItemToolTipWidget = CreateWidget<UItemToolTipWidget>(GetWorld(), TooltipWidgetClass);
		if (ItemToolTipWidget)
		{
			SetToolTip(ItemToolTipWidget);
			ItemToolTipWidget->SetToolTipItem(ItemData);
		}
	}
}

FVector2D UItemWidget::CalculateIconSize(const FInventorySlot& ItemData) const
{
	int32 ItemWidth = ItemData.ItemInstance.GetWidth();
	int32 ItemHeight = ItemData.ItemInstance.GetHeight();
		
	return FVector2D(
		ItemWidth * BaseSlotSize * 0.8f, 
		ItemHeight * BaseSlotSize * 0.8f
	);
}

void UItemWidget::SetIconSize(const FVector2D& NewSize)
{
	if (!ValidateIconComponents())
	{
		return;
	}

	ConfigureSizeBox(NewSize);
	
	ConfigureCanvasSlot(ItemSzBox, NewSize);
	ConfigureCanvasSlot(ItemIcon, NewSize);
	
	// 레이아웃 강제 업데이트
	ForceLayoutUpdate();
	
	UE_LOG(LogTemp, Log, TEXT("SetIconSize completed: %s"), *NewSize.ToString());
}

void UItemWidget::SetWidgetGridPos(int32 PositionX, int32 PositionY)
{
	ItemInfo.GridX = PositionX;
	ItemInfo.GridY = PositionY;
}

void UItemWidget::SetWidgetPosition(int32 PositionX, int32 PositionY)
{
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasSlot->SetPosition(FVector2D(PositionX, PositionY));
	}
}

bool UItemWidget::MoveGridPosition(int32 DeltaX, int32 DeltaY)
{
	if (ItemInfo.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveGridPosition failed: ItemInfo is empty"));
		return false;
	}
	ItemInfo.GridX = DeltaX;
	ItemInfo.GridY = DeltaY;
	return true;
}

void UItemWidget::ClearItemInfo()
{
	ItemInfo.Clear();
	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(nullptr); 
	}
	
	if (ItemSzBox)
	{
		ItemSzBox->SetWidthOverride(0.0f);
		ItemSzBox->SetHeightOverride(0.0f);
	}
	
	// 투명도 복원
	SetRenderOpacity(1.0f); 

	// 툴팁 제거
	SetToolTip(nullptr); 
}

void UItemWidget::DestoryItemWidget()
{
	RemoveFromParent();
	SetVisibility(ESlateVisibility::Collapsed);
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
	UItemDragDropOperation* DragOperation = NewObject<UItemDragDropOperation>(this);
	
	// 드래그 정보 설정
	DragOperation->ItemData = ItemInfo;
	DragOperation->SourceWidget = this;
	DragOperation->ItemWidth = ItemInfo.ItemInstance.GetWidth();
	DragOperation->ItemHeight = ItemInfo.ItemInstance.GetHeight();
	DragOperation->DragType = static_cast<EItemDragDropType>(ItemDragDropState);
	// 드래그 시각적 위젯 생성 (원본의 복사본)
	UItemWidget* DragVisual = FInventoryUtils::CreateItemWidget(GetWorld(), &ItemInfo);
	DragVisual->SetItemDragDropState(ItemDragDropState);

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

bool UItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 아이템 드래그 오퍼레이션인지 확인
	UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!ItemDragOp)
		return false;
	
	// 자기 자신에게 드롭하는 경우 처리하지 않음
	if (ItemDragOp->SourceWidget == this)
	{
		// 자기 자신에게 드롭하는 것은 유효하지 않지만, 드래그를 취소하지 않음
		UE_LOG(LogTemp, Log, TEXT("ItemWidget::NativeOnDrop - Dropped on self - ignoring"));
		return true; 
	}
	
	// 부모 인벤토리 위젯 찾기 - 위젯 계층을 따라 올라가며 UMainInventory 찾기
	UMainInventory* ParentInventory = nullptr;
	UWidget* CurrentWidget = GetParent();
	while (CurrentWidget)
	{
		ParentInventory = Cast<UMainInventory>(CurrentWidget);
		if (IsValid(ParentInventory))
		{
			break;
		}
		CurrentWidget = CurrentWidget->GetParent();
	}
	
	if (!IsValid(ParentInventory))
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemWidget::NativeOnDrop - Parent inventory not found"));
		return false; 
	}

	// 드롭 위치를 그리드 좌표로 변환
	FVector2D LocalPosition = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	
	// 부모 인벤토리의 함수를 사용하여 그리드 좌표 계산
	FVector2D GridPosition = ParentInventory->GetGridPositionFromScreenPosition(LocalPosition);
	
	int32 DropGridX = FMath::FloorToInt(GridPosition.X);
	int32 DropGridY = FMath::FloorToInt(GridPosition.Y);
	
	UE_LOG(LogTemp, Log, TEXT("ItemWidget::NativeOnDrop - Item dropped at grid position: (%d, %d)"), DropGridX, DropGridY);

	// 소스 아이템과 타겟 아이템 정보
	UItemWidget* SourceWidget = Cast<UItemWidget>(ItemDragOp->SourceWidget);
	if (!IsValid(SourceWidget))
	{
		return false;
	}

	// 여기서 드롭된 좌표(DropGridX, DropGridY)를 사용하여 필요한 로직 수행
	// 예: 아이템 교환, 위치 변경 등

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
	
	UE_LOG(LogTemp, Log, TEXT("Item swap completed successfully"));
	return true;
}

void UItemWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	
	// 드래그 취소 시 원본 위젯 복원
	SetRenderOpacity(1.0f);
	
	// 커스텀 뷰포트 클라이언트에 드래그 종료 알림
	if (GEngine && GEngine->GameViewport)
	{
		if (UDiaCustomGameViewPort* CustomViewport = Cast<UDiaCustomGameViewPort>(GEngine->GameViewport))
		{
			CustomViewport->OnDragEnd_CreateCautionWidget();
		}
	}
}

//아이템 사이즈 바꾸는거 디버깅용함수
bool UItemWidget::ValidateIconComponents() const
{
	if (!ItemIcon)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemIcon is null!"));
		return false;
	}
	
	if (!ItemSzBox)
	{
		UE_LOG(LogTemp, Error, TEXT("ItemSzBox is null! Check Blueprint binding."));
		return false;
	}
	
	return true;
}

void UItemWidget::ConfigureSizeBox(const FVector2D& NewSize)
{
	ItemSzBox->SetWidthOverride(NewSize.X);
	ItemSzBox->SetHeightOverride(NewSize.Y);
	
	ItemSzBox->SetMinDesiredWidth(NewSize.X);
	ItemSzBox->SetMinDesiredHeight(NewSize.Y);
	ItemSzBox->SetMaxDesiredWidth(NewSize.X);
	ItemSzBox->SetMaxDesiredHeight(NewSize.Y);
}

void UItemWidget::ConfigureCanvasSlot(UWidget* Widget, const FVector2D& NewSize)
{
	if (!Widget || !Widget->Slot)
	{
		return;
	}
	
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
	{
		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetSize(NewSize);
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
		CanvasSlot->SetAlignment(FVector2D(0.0f, 0.0f));
	}
}

void UItemWidget::ForceLayoutUpdate()
{
	InvalidateLayoutAndVolatility();
	
	if (UWidget* Parent = GetParent())
	{
		Parent->InvalidateLayoutAndVolatility();
	}
}
