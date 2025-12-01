// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "GameMode/DungeonGameMode.h"

#include "UI/HUDWidget.h"
#include "UI/Item/ItemWidget.h"
#include "UI/Inventory/MainInventory.h"
#include "Utils/InventoryUtils.h"

UDiaInventoryComponent::UDiaInventoryComponent()
	:InventoryGrid(GridWidth, GridHeight)
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UDiaInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OnItemRemoved.AddDynamic(this, &UDiaInventoryComponent::HandleItemRemoved);
}

void UDiaInventoryComponent::HandleItemRemoved(const FGuid& ItemID)
{
	//Inventory UI를 가져온다.
	ADungeonGameMode* GM = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;
	UHUDWidget* HUD = GM->GetHUDWidget();
	if (!HUD) return;
	UMainInventory* InvenWidget = HUD->GetInventoryWidget();
	if (!InvenWidget) return;
	//아이템을 제거한다.

	RemoveItem(ItemID, InvenWidget);
}

bool UDiaInventoryComponent::TryAddItem(const FInventorySlot& ItemData, UMainInventory* InvenWidget)
{
	if (!InvenWidget) return false;

	int32 InvenIdx = 0;
	int32 OutPosX = 0;
	int32 OutPosY = 0;

	//자동으로 찾는 함수 호출
	if (!FInventoryUtils::FindPlaceForItem(this, ItemData.ItemInstance.GetWidth(),
		ItemData.ItemInstance.GetHeight(), OutPosX, OutPosY)) 
		return false;

	bool res = InvenWidget->AddItem(ItemData, nullptr, OutPosY, OutPosX);

	if (res)
	{
		FillGrid(ItemData.ItemInstance.GetWidth(), ItemData.ItemInstance.GetHeight(), OutPosX, OutPosY);
	}

	return res;
}

bool UDiaInventoryComponent::RequestMoveItem(const FGuid& InstanceID, int32 DestX, int32 DestY, UMainInventory* InventoryWidget)
{
	const FInventorySlot* ItemInfo = InventoryWidget->GetItemDataAtGuid(InstanceID);
	if (!ItemInfo)
	{
#ifdef UE_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("MoveItem: Item with InstanceID %s not found."), *InstanceID.ToString());
#endif // UE_EDITOR
		return false;
	}

	ClearGrid(ItemInfo->ItemInstance.GetWidth(), ItemInfo->ItemInstance.GetHeight(), ItemInfo->GridX, ItemInfo->GridY);
	FillGrid(ItemInfo->ItemInstance.GetWidth(), ItemInfo->ItemInstance.GetHeight(), DestX, DestY);

	if (UItemWidget* ItemWidget = InventoryWidget->GetItemWidgetAtGuid(InstanceID))
	{
		InventoryWidget->MoveContainItem(InstanceID, DestX, DestY);
		ItemWidget->SetRenderOpacity(1.0f);
		InventoryWidget->UpdateItemPosition(ItemWidget, DestX, DestY);
#ifdef UE_EDITOR
		UE_LOG(LogTemp, Log, TEXT("Item moved to new position: (%d, %d)"), DestX, DestY);
#endif
		return true;
	}

	return false;
}

bool UDiaInventoryComponent::RemoveItem(const FGuid& InstanceID, UMainInventory* InvenWidget)
{
	const FInventorySlot* ItemInfo = InvenWidget->GetItemDataAtGuid(InstanceID);
	int32 ItemWidth = ItemInfo->ItemInstance.GetWidth();
	int32 ItemHeight = ItemInfo->ItemInstance.GetHeight();
	int32 PosX = ItemInfo->GridX;
	int32 PosY = ItemInfo->GridY;

	if (ClearGrid(ItemWidth, ItemHeight, PosX, PosY))
	{
		if (InvenWidget->RemoveContainItem(InstanceID))
		{
			UE_LOG(LogTemp, Log, TEXT("Item successfully removed from inventory: %s"), *InstanceID.ToString());
			return true;
		}
	}
	return false;
}

void UDiaInventoryComponent::FillGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
	// 아이템이 차지하는 그리드 공간을 채움
	for (int32 y = PosY; y < PosY + ItemHeight; ++y)
	{
		for (int32 x = PosX; x < PosX + ItemWidth; ++x)
		{
			InventoryGrid[y * GridWidth + x] = true;
		}
	}
}

bool UDiaInventoryComponent::CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY) const
{
	// 기본 범위 검사
	if (PosX < 0 || PosY < 0 || 
		PosX + ItemWidth > GetGridWidth() || 
		PosY + ItemHeight > GetGridHeight())
	{
#ifdef UE_EDITOR
		UE_LOG(LogTemp, Warning, TEXT("Cannot place item at (%d, %d): Out of bounds."), PosX, PosY);
#endif // UE_EDITOR
		return false;
	}

	// 그리드에서 셀 점유 상태 확인
	for (int32 Y = PosY; Y < PosY + ItemHeight; ++Y)
	{
		for (int32 X = PosX; X < PosX + ItemWidth; ++X)
		{
			int32 CellIndex = Y * GetGridWidth() + X;
			if (CellIndex >= 0 && CellIndex < InventoryGrid.Cells.Num())
			{
				if (InventoryGrid.Cells[CellIndex]) // 셀이 이미 점유됨
				{
#ifdef UE_EDITOR
					UE_LOG(LogTemp, Warning, TEXT("Cannot place item at (%d, %d) : Cell(% d, % d) is already occupied."), PosX, PosY, X, Y);
#endif // UE_EDITOR
					return false;
				}
			}
		}
	}

	return true;
}

bool UDiaInventoryComponent::FindPlaceForItem(int32 ItemWidth, int32 ItemHeight, int32& OutPosX, int32& OutPosY) const
{
	// 좌상단부터 순차적으로 배치 가능한 위치 찾기
	for (int32 Y = 0; Y <= GetGridHeight() - ItemHeight; ++Y)
	{
		for (int32 X = 0; X <= GetGridWidth() - ItemWidth; ++X)
		{
			if (CanPlaceItemAt(ItemWidth, ItemHeight, X, Y))
			{
				OutPosX = X;
				OutPosY = Y;
				return true;
			}
		}
	}

	return false; // 배치할 수 있는 공간이 없음
}

bool UDiaInventoryComponent::ClearGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
	for( int32 y = PosY; y < PosY + ItemHeight; ++y)
	{
		for (int32 x = PosX; x < PosX + ItemWidth; ++x)
		{
			int32 CellIndex = y * GridWidth + x;
			if (CellIndex >= 0 && CellIndex < InventoryGrid.Cells.Num())
			{
				InventoryGrid.Cells[CellIndex] = false;
			}
			else
			{
#ifdef UE_EDITOR
				UE_LOG(LogTemp, Warning, TEXT("ClearGrid: CellIndex %d outof bounds."), CellIndex);
#endif // UE_EDITOR
					return false;
			}
		}
	}

	return true;
}
