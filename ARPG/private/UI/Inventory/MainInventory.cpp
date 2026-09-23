// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/MainInventory.h"
#include "UI/Item/ItemWidget.h"
#include "UI/DragDrop/ItemDragDropOperation.h"
#include "UI/Inventory/EquipWidget.h"
#include "UI/Inventory/EquipSlot.h"
#include "UI/Inventory/DiaInventoryPage.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

#include "System/ItemSubsystem.h"
#include "System/DiaSaveGame.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Utils/InventoryUtils.h"
#include "Logging/ARPGLogChannels.h"

void UMainInventory::NativeConstruct()
{
	Super::NativeConstruct();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		ItemSubsystem = GameInstance->GetSubsystem<UItemSubsystem>();
	}
}

void UMainInventory::NativeDestruct()
{
	if (InventoryComponent.IsValid())
	{
		InventoryComponent->OnItemAdded.RemoveDynamic(this, &ThisClass::HandleItemAdded);
		InventoryComponent->OnItemRemoved.RemoveDynamic(this, &ThisClass::HandleItemRemoved);
		InventoryComponent->OnItemMoved.RemoveDynamic(this, &ThisClass::HandleItemMoved);
		InventoryComponent->OnGoldChanged.RemoveDynamic(this, &ThisClass::HandleGoldChanged);
	}

	Super::NativeDestruct();
}

void UMainInventory::InitializeInventory()
{

	// 초기 상태는 Hidden으로 설정
	if (!ItemSubsystem)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			ItemSubsystem = GameInstance->GetSubsystem<UItemSubsystem>();
		}
	}

	if (!InventoryComponent.IsValid())
	{
		return;
	}

	GridWidth = InventoryComponent->GetGridWidth();
	GridHeight = InventoryComponent->GetGridHeight();

	if (InventorySlots.IsEmpty())
	{
		CreateInventory();
	}

	// Reconcile events missed while the widget was destructed.
	TArray<FGuid> DisplayedItemIDs;
	ItemWidgets.GetKeys(DisplayedItemIDs);
	for (const FGuid& ItemID : DisplayedItemIDs)
	{
		if (!InventoryComponent->GetItemDataAtGuid(ItemID))
		{
			RemoveContainItem(ItemID);
		}
	}
	for (const TPair<FGuid, FInventorySlot>& ItemPair : InventoryComponent->GetInventoryItems())
	{
		if (UItemWidget* ItemWidget = GetItemWidgetAtGuid(ItemPair.Key); IsValid(ItemWidget))
		{
			ItemWidget->SetItemInfo(ItemPair.Value);
			UpdateItemPosition(ItemWidget, ItemPair.Value.GridX, ItemPair.Value.GridY);
		}
		else
		{
			ItemWidgets.Remove(ItemPair.Key);
			HandleItemAdded(ItemPair.Value);
		}
	}
	UpdateGoldText(InventoryComponent->GetGold());
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

//여기서, 아이템을 추가하며 ItemWidget을 생성
bool UMainInventory::AddItemToInventory(const FInventorySlot& ItemData, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
	// 범위 체크 추가
	if (!FInventoryUtils::IsValidGridPosition(GridWidth, GridHeight, ItemWidth, ItemHeight, PosX, PosY))
		return false;
	//이미 존재하는 아이템인지 확인
	if (ItemWidgets.Find(ItemData.ItemInstance.InstanceID))
	{
		//스택형 아이템인지 체크 해야한다.
		//스택형 아이템이 아니라면 그냥 새롭게 추가해야한다.
		if(ItemSubsystem && ItemSubsystem->IsItemStackable(ItemData.ItemInstance))
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

	//아이템 위젯 생성
	//근데 그냥 만들면 안되나??;

	//아이템 위잿 생성

	// ItemSubsystem에서 아이템 위젯을 생성한다.
	UItemWidget* ItemWidget = FInventoryUtils::CreateItemWidget(this, &ItemData);
	if (!IsValid(ItemWidget))
	{
		return false;
	}
	ItemWidget->SetItemDragDropState(static_cast<int32>(EItemDragDropType::EIDT_Inventory));
	if (IsValid(ItemWidget))
	{
		// ItemWidget을 InventoryCanvas의 자식으로 추가
		UCanvasPanelSlot* ItemSlot = InventoryCanvas->AddChildToCanvas(ItemWidget);

		if (IsValid(ItemSlot))
		{
			// 해당 슬롯과 같은 위치와 크기로 설정
			FVector2D SlotSize = FVector2D(szSlot * ItemWidth, szSlot * ItemHeight);
			FVector2D SlotPosition = InventorySlots[LeftTopSlot]->GetPosition();

			ItemSlot->SetPosition(SlotPosition);
			ItemSlot->SetSize(SlotSize);   // 명시적 크기 설정
			
			// 아이템 위젯을 맵에 저장
			ItemWidgets.Emplace(ItemData.ItemInstance.InstanceID, ItemWidget);
			ItemWidget->SetWidgetGridPos(PosX, PosY);

			ItemWidget->OnItemRightClicked.BindLambda([this](UItemWidget* ClickedItemWidget)
			{
					if (!IsValid(ClickedItemWidget) || !IsValid(ItemSubsystem))
					{
						return;
					}
					const FItemInstance& ItemInstance = ClickedItemWidget->GetItemInfo().ItemInstance;
					EEquipmentSlot eEquipSlot = ItemSubsystem->GetEquipmentSlot(ItemInstance);
					if (UDiaInventoryPage* Page = InventoryPage.Get())
					{
						if (UEquipWidget* EquipWidget = Page->GetEquipmentWidget())
						{
							UEquipSlot* EquipSlotWidget = EquipWidget->GetEquipSlot(eEquipSlot);
							if (IItemContainer* TargetContainer = Cast<IItemContainer>(EquipSlotWidget))
							{
								TargetContainer->AddItem(ClickedItemWidget->GetItemInfo(), ClickedItemWidget);
							}
						}
					}
			});
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

	const FGuid RemovedItemID = ItemWidget->GetItemInfo().ItemInstance.InstanceID;
	ItemWidget->ClearItemInfo();

	// ItemWidget을 InventoryCanvas에서 제거
	InventoryCanvas->RemoveChild(ItemWidget);

	ItemWidget->RemoveFromParent();
	ItemWidgets.Remove(RemovedItemID);
	ItemWidget->Destruct();
	ItemWidget = nullptr;

	return true;
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
			const int32 ItemWidth = ItemSubsystem ? ItemSubsystem->GetItemWidth(ItemData.ItemInstance) : 1;
			const int32 ItemHeight = ItemSubsystem ? ItemSubsystem->GetItemHeight(ItemData.ItemInstance) : 1;
			if (ItemData.GridX <= GridX && GridX < ItemData.GridX + ItemWidth &&
				ItemData.GridY <= GridY && GridY < ItemData.GridY + ItemHeight)
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

UItemWidget* UMainInventory::GetItemWidgetAtGuid(const FGuid& ItemInstanceID) const
{
	if (UItemWidget* const* FoundWidget = ItemWidgets.Find(ItemInstanceID))
	{
		return *FoundWidget;
	}
	return nullptr;
}

const FInventorySlot* UMainInventory::GetItemDataAtGuid(const FGuid& ItemInstanceID) const
{
	if (UItemWidget* const* FoundWidget = ItemWidgets.Find(ItemInstanceID))
	{
		return &(*FoundWidget)->GetItemInfo();
	}
	return nullptr;
}

void UMainInventory::CreateInventory()
{
	// 위젯 클래스 한 번만 로드
	FSoftObjectPath ItemWidgetPath(TEXT("/Game/UI/Inventory/WBP_InvenSlot.WBP_InvenSlot_C"));
	TSoftClassPtr<UUserWidget> WidgetAssetPtr(ItemWidgetPath);
	UClass* ItemWidgetClass = WidgetAssetPtr.LoadSynchronous();

	if (!ItemWidgetClass)
	{
		UE_LOG(LogARPG, Error, TEXT("Failed to load ItemWidget class"));
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
	if (!IsValid(ItemDragOp) || !IsValid(ItemDragOp->SourceWidget))
	{
		return false;
	}

	FVector2D ScreenPosition = InDragDropEvent.GetScreenSpacePosition();
	const FVector2D CanvasLocalPosition = GetCanvasLocalPositionFromScreenPosition(InGeometry, ScreenPosition);
	const int32 NewGridX = FMath::FloorToInt(CanvasLocalPosition.X / szSlot);
	const int32 NewGridY = FMath::FloorToInt(CanvasLocalPosition.Y / szSlot);

	bool bStopDrop = false;
	const bool bCheckResult = CheckInventoryDrop(ItemDragOp, NewGridX, NewGridY, ScreenPosition, bStopDrop);
	const bool bSuccess = bStopDrop ? bCheckResult : AddItem(ItemDragOp->ItemData, ItemDragOp->SourceWidget, NewGridY, NewGridX);
	if (!bSuccess && IsValid(ItemDragOp->SourceWidget))
	{
		ItemDragOp->SourceWidget->SetRenderOpacity(1.0f);
	}
	return bSuccess;
}

bool UMainInventory::CheckInventoryDrop(UItemDragDropOperation* ItemDragOp, int32 NewGridX, int32 NewGridY, FVector2D& ScreenPosition, bool& retFlag)
{
	// UI checks bounds only. Components validate occupancy, including overlap with the moving item.
	retFlag = true;
	if (!IsValid(ItemDragOp) || !InventoryComponent.IsValid() || IsDropOutsideAllWidgets(ScreenPosition))
	{
		return false;
	}
	if (!FInventoryUtils::IsValidGridPosition(GridWidth, GridHeight, ItemDragOp->ItemWidth, ItemDragOp->ItemHeight, NewGridX, NewGridY))
	{
		return false;
	}
	retFlag = false;
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
		UE_LOG(LogARPG, Error, TEXT("InventoryCanvas is not valid"));
		return FVector2D::ZeroVector;
	}
		
	// InventoryCanvas의 오프셋을 고려한 실제 로컬 좌표 계산
	//main에서 해당 위치만큼 떨어져 있으니까 더하기.
	FVector2D CanvasOffset = MainWidgetGeometry.AbsoluteToLocal(InventoryCanvas->GetCachedGeometry().GetAbsolutePosition());
	FVector2D CanvasLocalPosition = MainLocalPosition - CanvasOffset;
	
	UE_LOG(LogARPG, VeryVerbose, TEXT("MainLocal: (%f, %f), CanvasOffset: (%f, %f), CanvasLocal: (%f, %f)"), 
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

bool UMainInventory::AddItem(const FInventorySlot& Item, UItemWidget* ItemWidget, int32 PosY, int32 PosX)
{
	if (!InventoryComponent.IsValid() || Item.IsEmpty())
	{
		return false;
	}

	// Resolve ownership from component data, not the item's equipment category (left/right rings).
	const FGuid ItemID = Item.ItemInstance.InstanceID;
	if (IsValid(ItemWidget) && ItemWidget->GetItemInstanceID() != ItemID)
	{
		return false;
	}
	if (InventoryComponent->GetItemDataAtGuid(ItemID))
	{
		if (PosX == -1 && PosY == -1)
		{
			return true; // Already in this inventory; no position was requested.
		}
		return InventoryComponent->RequestMoveItem(ItemID, PosX, PosY);
	}
	if (EquippementComponent.IsValid())
	{
		EEquipmentSlot SourceSlot = EEquipmentSlot::EES_None;
		for (const auto& Entry : EquippementComponent->GetEquipmentMap())
		{
			if (Entry.Value.ItemInstance.InstanceID == ItemID)
			{
				SourceSlot = Entry.Key;
				break;
			}
		}
		if (SourceSlot != EEquipmentSlot::EES_None)
		{
			FInventorySlot ReturnedItem;
			return EquippementComponent->TryUnEquipToInventory(SourceSlot, ReturnedItem, PosX, PosY);
		}
	}
	return false;
}
bool UMainInventory::RemoveContainItem(const FGuid& ItemInstanceID)
{
	UItemWidget* ItemWidget = GetItemWidgetAtGuid(ItemInstanceID);
	if (!IsValid(ItemWidget))
		return false;

	ItemWidget->ClearItemInfo();

	// ItemWidget을 InventoryCanvas에서 제거
	InventoryCanvas->RemoveChild(ItemWidget);

	ItemWidget->RemoveFromParent();
	ItemWidgets.Remove(ItemInstanceID);
	ItemWidget->Destruct();
	ItemWidget = nullptr;

	return true;

}

bool UMainInventory::MoveContainItem(const FGuid& ItemInstanceID, int32 NewPosX, int32 NewPosY)
{
	UItemWidget* ItemWidget = GetItemWidgetAtGuid(ItemInstanceID);
	if (!IsValid(ItemWidget))
	{
		return false;
	}
	ItemWidget->MoveGridPosition(NewPosX, NewPosY);
	return true;
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
	
	// 페이지에 저장된 장착 위젯 참조로 경계 체크
	bool bInsideEquipment = false;
	if (UDiaInventoryPage* Page = InventoryPage.Get())
	{
		UEquipWidget* EquipWidget = Page->GetEquipmentWidget();
		if (IsValid(EquipWidget))
		{
			FGeometry EquipGeometry = EquipWidget->GetCachedGeometry();
			FVector2D EquipLocalPosition = EquipGeometry.AbsoluteToLocal(ScreenPosition);
			FVector2D EquipSize = EquipGeometry.GetLocalSize();

			// 장착 위젯 경계 내부인지 체크
			bInsideEquipment = (EquipLocalPosition.X >= 0 && EquipLocalPosition.X <= EquipSize.X &&
			                   EquipLocalPosition.Y >= 0 && EquipLocalPosition.Y <= EquipSize.Y);
		}
	}
	
	// 둘 다 바깥이면 true 반환 (완전히 바깥으로 드롭됨)
	return (!bInsideInventory && !bInsideEquipment);
}

void UMainInventory::SetInventoryPage(UDiaInventoryPage* InInventoryPage)
{
	InventoryPage = InInventoryPage;
}

void UMainInventory::SetInventoryComponent(UDiaInventoryComponent* InComponent)
{
	if (InventoryComponent.IsValid())
	{
		InventoryComponent->OnItemAdded.RemoveDynamic(this, &ThisClass::HandleItemAdded);
		InventoryComponent->OnItemRemoved.RemoveDynamic(this, &ThisClass::HandleItemRemoved);
		InventoryComponent->OnItemMoved.RemoveDynamic(this, &ThisClass::HandleItemMoved);
		InventoryComponent->OnGoldChanged.RemoveDynamic(this, &ThisClass::HandleGoldChanged);
	}

	InventoryComponent = InComponent;

	if (InventoryComponent.IsValid())
	{
		InventoryComponent->OnItemAdded.AddUniqueDynamic(this, &ThisClass::HandleItemAdded);
		InventoryComponent->OnItemRemoved.AddUniqueDynamic(this, &ThisClass::HandleItemRemoved);
		InventoryComponent->OnItemMoved.AddUniqueDynamic(this, &ThisClass::HandleItemMoved);
		InventoryComponent->OnGoldChanged.AddUniqueDynamic(this, &UMainInventory::HandleGoldChanged);
		UpdateGoldText(InventoryComponent->GetGold());
	}
}

void UMainInventory::SetEquipmentComponent(UDiaEquipmentComponent* InComponent)
{
	EquippementComponent = InComponent;
}

void UMainInventory::HandleGoldChanged(int32 NewGoldAmount, int32 DeltaAmount)
{
	UpdateGoldText(NewGoldAmount);
}

void UMainInventory::HandleItemAdded(const FInventorySlot& Item)
{
	if (!ItemWidgets.Contains(Item.ItemInstance.InstanceID))
	{
		const int32 ItemWidth = ItemSubsystem ? ItemSubsystem->GetItemWidth(Item.ItemInstance) : 1;
		const int32 ItemHeight = ItemSubsystem ? ItemSubsystem->GetItemHeight(Item.ItemInstance) : 1;
		AddItemToInventory(Item, ItemWidth, ItemHeight, Item.GridX, Item.GridY);
	}
}

void UMainInventory::HandleItemRemoved(const FGuid& ItemID)
{
	if (ItemWidgets.Contains(ItemID))
	{
		RemoveContainItem(ItemID);
	}
}

void UMainInventory::HandleItemMoved(const FGuid& ItemID, int32 GridX, int32 GridY)
{
	if (UItemWidget* ItemWidget = GetItemWidgetAtGuid(ItemID))
	{
		MoveContainItem(ItemID, GridX, GridY);
		UpdateItemPosition(ItemWidget, GridX, GridY);
		ItemWidget->SetRenderOpacity(1.0f);
	}
}

void UMainInventory::UpdateGoldText(int32 NewGoldAmount)
{
	if (GoldText)
	{
		GoldText->SetText(FText::AsNumber(NewGoldAmount));
	}
}
