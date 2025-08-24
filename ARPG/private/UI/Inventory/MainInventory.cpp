// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/MainInventory.h"
#include "UI/Item/ItemWidget.h"
#include "UI/DragDrop/ItemDragDropOperation.h"
#include "UI/HUDWidget.h"
#include "UI/Inventory/EquipWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/Image.h"

#include "System/ItemSubsystem.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Utils/InventoryUtils.h"

void UMainInventory::NativeConstruct()
{
	Super::NativeConstruct();

}

void UMainInventory::InitializeInventory()
{

	// 초기 상태는 Hidden으로 설정
	SetVisibility(ESlateVisibility::Hidden);

	GridWidth = InventoryComponent->GetGridWidth();
	GridHeight = InventoryComponent->GetGridHeight();

	CreateInventory();
}

bool UMainInventory::IsSlotEmpty(int32 SlotIndex) const
{
	if (!InventorySlots.IsValidIndex(SlotIndex))
		return false;
	
	UUserWidget* ItemWidget = GetItemWidgetAt(SlotIndex);
	if (!ItemWidget)
		return true;
	
	// ItemWidget이 있지만 아이템 정보가 없거나 숨겨진 경우
	return ItemWidget->GetVisibility() != ESlateVisibility::Visible;
}

//여기서, 아이템을 추가하며 ItemWidget을 생성하고, 아이템이 차지하는 공간을 조정하는 로직을 구현한다.
bool UMainInventory::AddItemToInventory(const FInventorySlot& ItemData, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
	// 범위 체크 추가
	if (!FInventoryUtils::CanPlaceItemAt(InventoryComponent.Get(), ItemWidth, ItemHeight, PosX, PosY))
		return false;
	//이미 존재하는 아이템인지 확인
	if (ItemWidgets.Find(ItemData.ItemInstance.InstanceID))
	{
		//스택형 아이템인지 체크 해야한다.
		//스택형 아이템이 아니라면 그냥 새롭게 추가해야한다.
		if(ItemData.ItemInstance.BaseItem.bStackable)
		{
			////스택형 아이템이라면, 해당 아이템의 스택을 증가시키는 로직을 구현해야 한다.
			//UItemWidget* ExistingItemWidget = GetItemWidgetBySlotIndex(PosY * 10 + PosX);
			//if (ExistingItemWidget)
			//{
			//	ExistingItemWidget->IncreaseStack(ItemData.StackCount);
			//	return true;
			//}
			return false;
		}
	}

	// 첫 번째 슬롯에만 아이템 위젯 생성하고 크기 조정
	int32 LeftTopSlot = PosY * GridWidth + PosX;
	int32 RightBottomSlot = (PosY + ItemHeight) * GridWidth + (PosX + ItemWidth);

	//아이템 위젯 생성
	//근데 그냥 만들면 안되나??;

	//아이템 위잿 생성

	// ItemSubsystem에서 아이템 위젯을 생성한다.
	UItemWidget* ItemWidget = FInventoryUtils::CreateItemWidget(this, ItemData);
	if (IsValid(ItemWidget))
	{
		// ItemWidget을 InventoryCanvas의 자식으로 추가
		UCanvasPanelSlot* ItemSlot = InventoryCanvas->AddChildToCanvas(ItemWidget);

		if (IsValid(ItemSlot))
		{
			// 해당 슬롯과 같은 위치와 크기로 설정
			FVector2D SlotSize = FVector2D(szSlot * ItemWidth, szSlot * ItemHeight);
			FVector2D SlotPosition = (InventorySlots[LeftTopSlot]->GetPosition() + InventorySlots[RightBottomSlot]->GetPosition()) * 0.5f - (SlotSize * 0.5f);

			ItemSlot->SetPosition(SlotPosition);
			ItemSlot->SetSize(SlotSize);   // 명시적 크기 설정
						
			UE_LOG(LogTemp, Log, TEXT("MainInventory: Canvas Panel Slot configured - Position: %s, Size: %s"), 
				*SlotPosition.ToString(), *SlotSize.ToString());
			
			// 아이템 위젯을 맵에 저장
			ItemWidgets.Emplace(ItemData.ItemInstance.InstanceID, ItemWidget);
			ItemWidget->SetWidgetGridPos(PosX, PosY);
			
			//결과에 대한 로그 작성
			//fguid, name, gridx ,gridy 표현
			FString guid = ItemData.ItemInstance.InstanceID.ToString();
			UE_LOG(LogTemp, Log, TEXT("Added item to inventory: %s (guid : %s) at Slot (%f, %f)"), *ItemData.ItemInstance.BaseItem.ItemID.ToString(), *guid, SlotPosition.X, SlotPosition.Y);
		}
	}

	return true;
}

bool UMainInventory::RemoveItemFromInventory(int32 SlotIndex)
{
	// 슬롯 인덱스 유효성 검사
	//SlotIndex를 Gridx, Gridy로 변환해서

	int32 GridX = SlotIndex % GridWidth;
	int32 GridY = SlotIndex / GridWidth;
	UItemWidget* ItemWidget = GetItemWidgetAtGridPosition(GridX, GridY);
	if (!IsValid(ItemWidget))
		return false;

	// ItemWidget을 InventoryCanvas에서 제거
	InventoryCanvas->RemoveChild(ItemWidget);

	ItemWidget->RemoveFromViewport();
	ItemWidget->RemoveFromParent();
	ItemWidgets.Remove(Cast<UItemWidget>(ItemWidget)->GetItemInfo().ItemInstance.InstanceID);
	ItemWidget->Destruct();
	ItemWidget = nullptr;

	return true;
}

UItemWidget* UMainInventory::GetItemWidgetBySlotIndex(FGuid guid) const
{
	if (UItemWidget* const* FoundWidget = ItemWidgets.Find(guid))
	{
		return *FoundWidget;
	}
	return nullptr;
}

UItemWidget* UMainInventory::GetItemWidgetAtGridPosition(int32 GridX, int32 GridY) const
{
	// 모든 아이템 위젯을 순회하여 해당 위치에 있는 아이템 찾기
	for (const auto& ItemPair : ItemWidgets)
	{
		UItemWidget* ItemWidget = ItemPair.Value;
		if (IsValid(ItemWidget))
		{
			const FInventorySlot& ItemData = ItemWidget->GetItemInfo();
			// 아이템이 해당 그리드 위치를 차지하는지 확인
			if (ItemData.GridX <= GridX && GridX < ItemData.GridX + ItemData.ItemInstance.GetWidth() &&
				ItemData.GridY <= GridY && GridY < ItemData.GridY + ItemData.ItemInstance.GetHeight())
			{
				return ItemWidget;
			}
		}
	}
	return nullptr;
}

UItemWidget* UMainInventory::GetItemWidgetAt(int32 SlotIndex) const
{
	int32 GridX = SlotIndex % GridWidth;
	int32 GridY = SlotIndex / GridWidth;
	return GetItemWidgetAtGridPosition(GridX, GridY);
}

void UMainInventory::CreateInventory()
{
	// 위젯 클래스 한 번만 로드
	FSoftObjectPath ItemWidgetPath(TEXT("/Game/UI/Inventory/WBP_InvenSlot.WBP_InvenSlot_C"));
	TSoftClassPtr<UUserWidget> WidgetAssetPtr(ItemWidgetPath);
	UClass* ItemWidgetClass = WidgetAssetPtr.LoadSynchronous();

	if (!ItemWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ItemWidget class"));
		return;
	}
	
	for (int32 y = 0; y < GridHeight; ++y)
	{
		for (int32 x = 0; x < GridWidth; ++x)
		{
			UUserWidget* ItemWidget = WidgetTree->ConstructWidget<UUserWidget>(ItemWidgetClass);
			
			if (ItemWidget)
			{
				UCanvasPanelSlot* NewSlot = InventoryCanvas->AddChildToCanvas(ItemWidget);
				int32 SlotIndex = y * GridWidth + x;
				ConfigInventorySlot(SlotIndex, NewSlot);
				InventorySlots.Add(NewSlot);

				//초기에는 일단 숨긴다 (나중에 상호작요을 통해 보여줄 예정)
				//ItemWidget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void UMainInventory::ConfigInventorySlot(int32 SlotIndex, UCanvasPanelSlot* CanvasSlot)
{
	if (CanvasSlot)
	{
		FVector2D Position = FVector2D((SlotIndex % GridWidth) * szSlot, (SlotIndex / GridWidth) * szSlot);
		FVector2D Size = FVector2D(szSlot, szSlot);
		CanvasSlot->SetPosition(Position);
		CanvasSlot->SetSize(Size);
	}
}

void UMainInventory::GetAllItemWidgets(TArray<UUserWidget*>& OutItemWidgets) const
{
	OutItemWidgets.Empty();
	
	for (UCanvasPanelSlot* ItemSlot : InventorySlots)
	{
		if (IsValid(ItemSlot))
		{
			UWidget* ContentWidget = ItemSlot->GetContent();
			UUserWidget* ItemWidget = Cast<UUserWidget>(ContentWidget);
			if (IsValid(ItemWidget))
			{
				OutItemWidgets.Add(ItemWidget);
			}
		}
	}	
}

bool UMainInventory::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!ItemDragOp)
		return false;
	
	// 화면 좌표를 InventoryCanvas 기준의 로컬 좌표로 변환
	FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
	FVector2D CanvasLocalPosition = GetCanvasLocalPositionFromScreenPosition(InGeometry, ScreenPosition);
	
	// 그리드 좌표로 변환
	int32 NewGridX = FMath::FloorToInt(CanvasLocalPosition.X / szSlot);
	int32 NewGridY = FMath::FloorToInt(CanvasLocalPosition.Y / szSlot);

	//UE_LOG(LogTemp, Log, TEXT("MainInventory::NativeOnDrop - Screen: (%f, %f), CanvasLocal: (%f, %f), Grid: (%d, %d)"),
	//	ScreenPosition.X, ScreenPosition.Y, CanvasLocalPosition.X, CanvasLocalPosition.Y, NewGridX, NewGridY);
	
	// 인벤토리 슬롯 내의 유효한 위치인지 확인
	if (InventoryComponent.IsValid() && 
		!InventoryComponent->CanPlaceItemAt(ItemDragOp->ItemWidth, ItemDragOp->ItemHeight, NewGridX, NewGridY))
	{
		// 유효하지 않은 위치 - 원래 위치로 복원
		if (ItemDragOp->SourceWidget)
		{
			ItemDragOp->SourceWidget->SetRenderOpacity(1.0f);
		}
		//인벤토리 바깥인지 체크해야 한다.
		
		// 드롭된 위치가 인벤토리와 장착 위젯 모두의 바깥인지 체크
		if (IsDropOutsideAllWidgets(ScreenPosition))
		{
			UE_LOG(LogTemp, Warning, TEXT("Item dropped completely outside all widgets - cancelling drop"));
			return false;
		}

		//UE_LOG(LogTemp, Warning, TEXT("Invalid drop position - failed validation"));
		return false;
	}
	
	// 드롭 위치에 기존 아이템이 있는지 확인
	UItemWidget* ExistingItem = GetItemWidgetAtGridPosition(NewGridX, NewGridY);
	if (ExistingItem && ExistingItem != ItemDragOp->SourceWidget)
	{
		// 다른 아이템 위에 드롭하는 경우 - ItemWidget으로 이벤트 전파
		UE_LOG(LogTemp, Log, TEXT("Dropping on existing item - letting ItemWidget handle it"));
		return false; 
	}
	
	// 빈 공간에 드롭하는 경우 - 직접 처리
	// 추가 충돌 검사 (멀티 그리드 아이템의 경우)
	for (int32 CheckY = NewGridY; CheckY < NewGridY + ItemDragOp->ItemHeight; ++CheckY)
	{
		for (int32 CheckX = NewGridX; CheckX < NewGridX + ItemDragOp->ItemWidth; ++CheckX)
		{
			UItemWidget* ExistingItemInArea = GetItemWidgetAtGridPosition(CheckX, CheckY);
			if (ExistingItemInArea && ExistingItemInArea != ItemDragOp->SourceWidget)
			{
				// 다른 아이템과 충돌 - 드롭 거부
				if (ItemDragOp->SourceWidget)
				{
					ItemDragOp->SourceWidget->SetRenderOpacity(1.0f);
				}
				UE_LOG(LogTemp, Warning, TEXT("Item collision detected at (%d, %d)"), CheckX, CheckY);
				return false;
			}
		}
	}
	
	// 아이템 위치 업데이트
	if (ItemDragOp->SourceWidget)
	{
		// 원본 위젯의 투명도 복원
		ItemDragOp->SourceWidget->SetRenderOpacity(1.0f);
		
		// 새로운 위치로 이동
		UpdateItemPosition(ItemDragOp->SourceWidget, NewGridX, NewGridY);
		
		UE_LOG(LogTemp, Log, TEXT("Item moved to new position: (%d, %d)"), NewGridX, NewGridY);
	}
	
	return true;
}

bool UMainInventory::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(InOperation);
	if (!ItemDragOp)
		return false;
	
	// 드래그 오버 중 시각적 피드백 제공 가능
	// 예: 유효한 드롭 위치인지 하이라이트 표시
	
	return true;
}

FVector2D UMainInventory::GetGridPositionFromScreenPosition(const FVector2D& ScreenPosition) const
{
	// Canvas 내의 로컬 좌표를 그리드 좌표로 변환
	return FVector2D(
		ScreenPosition.X / szSlot,
		ScreenPosition.Y / szSlot
	);
}

//인벤토리 내부의 위치 판별 함수
//다른 위치로 이동할때는 또 다른 함수 호출해야함
FVector2D UMainInventory::GetCanvasLocalPositionFromScreenPosition(const FGeometry& MainWidgetGeometry, const FVector2D& ScreenPosition) const
{
	// MainInventory 위젯 기준의 로컬 좌표로 변환
	FVector2D MainLocalPosition = MainWidgetGeometry.AbsoluteToLocal(ScreenPosition);
	
	// InventoryCanvas가 문제 있으면 return 0;
	if (!IsValid(InventoryCanvas))
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryCanvas is not valid"));
		return FVector2D::ZeroVector;
	}
		
	// InventoryCanvas의 오프셋을 고려한 실제 로컬 좌표 계산
	//main에서 해당 위치만큼 떨어져 있으니까 더하기.
	FVector2D CanvasOffset = MainWidgetGeometry.AbsoluteToLocal(InventoryCanvas->GetCachedGeometry().GetAbsolutePosition());
	FVector2D CanvasLocalPosition = MainLocalPosition - CanvasOffset;
	
	UE_LOG(LogTemp, VeryVerbose, TEXT("MainLocal: (%f, %f), CanvasOffset: (%f, %f), CanvasLocal: (%f, %f)"), 
		MainLocalPosition.X, MainLocalPosition.Y, 
		CanvasOffset.X, CanvasOffset.Y,
		CanvasLocalPosition.X, CanvasLocalPosition.Y);
	
	return CanvasLocalPosition;
}

void UMainInventory::UpdateItemPosition(UItemWidget* ItemWidget, int32 NewGridX, int32 NewGridY)
{
	if (!IsValid(ItemWidget))
		return;
	
	// 아이템 위젯의 그리드 위치 업데이트
	ItemWidget->SetWidgetGridPos(NewGridX, NewGridY);
	
	// 캔버스 슬롯 위치 업데이트
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ItemWidget->Slot))
	{
		FVector2D NewPosition = FVector2D(NewGridX * szSlot, NewGridY * szSlot);
		CanvasSlot->SetPosition(NewPosition);
	}
}

bool UMainInventory::IsDropOutsideAllWidgets(const FVector2D& ScreenPosition) const
{
	// 현재 위젯(MainInventory)의 경계 체크
	FGeometry MyGeometry = GetCachedGeometry();
	FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(ScreenPosition);
	FVector2D MySize = MyGeometry.GetLocalSize();
	
	// 인벤토리 위젯 경계 내부인지 체크
	bool bInsideInventory = (LocalPosition.X >= 0 && LocalPosition.X <= MySize.X && 
	                        LocalPosition.Y >= 0 && LocalPosition.Y <= MySize.Y);
	
	// HUD를 통해 장착 위젯 참조 가져오기
	bool bInsideEquipment = false;
	UWidget* ParentWidget = GetParent();
	while (ParentWidget)
	{
		if (UHUDWidget* HUDWidget = Cast<UHUDWidget>(ParentWidget))
		{
			if (UEquipWidget* EquipWidget = HUDWidget->GetEquipmentWidget())
			{
				FGeometry EquipGeometry = EquipWidget->GetCachedGeometry();
				FVector2D EquipLocalPosition = EquipGeometry.AbsoluteToLocal(ScreenPosition);
				FVector2D EquipSize = EquipGeometry.GetLocalSize();
				
				// 장착 위젯 경계 내부인지 체크
				bInsideEquipment = (EquipLocalPosition.X >= 0 && EquipLocalPosition.X <= EquipSize.X && 
				                   EquipLocalPosition.Y >= 0 && EquipLocalPosition.Y <= EquipSize.Y);
			}
			break;
		}
		ParentWidget = ParentWidget->GetParent();
	}
	
	// 둘 다 바깥이면 true 반환 (완전히 바깥으로 드롭됨)
	return (!bInsideInventory && !bInsideEquipment);
}