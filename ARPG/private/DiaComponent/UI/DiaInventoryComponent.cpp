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
	PrimaryComponentTick.bCanEverTick = true;
}


void UDiaInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UDiaInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UDiaInventoryComponent::TryAddItem(const FInventoryItem& ItemData, UMainInventory* InvenWidget)
{
	if (!InvenWidget) return false;

	int32 InvenIdx = 0;
	int32 OutPosX = 0;
	int32 OutPosY = 0;

	//아이템을 넣을 수 있는 공간을 체크
	//없다면 false
	if (!FInventoryUtils::FindPlaceForItem(this, ItemData.Width, ItemData.Height, OutPosX, OutPosY)) return false;

	InvenIdx = OutPosX * GridWidth + OutPosY;
	//ui작업
	bool res = InvenWidget->AddItemToInventory(ItemData, ItemData.Width, ItemData.Height, OutPosX, OutPosY);
	//삽입에 성공하면, 여기서도 아이템이 삽입 되었다는 사실을 추가한다.
	if (res)
	{
		Items.Add(ItemData);
		FillGrid(ItemData.Width, ItemData.Height, OutPosX, OutPosY);
		
		return true;
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

bool UDiaInventoryComponent::RemoveItem(const FGuid& InstanceID, UMainInventory* InventoryWidget)
{
	//for (int32 i = 0; i < Items.Num(); i++)
	//{
	//	if (Items[i].InstanceID == InstanceID)
	//	{
	//		// 그리드에서 공간 비우기
	//		ClearGrid(Items[i].Width, Items[i].Height, ItemWidth, ItemHeight);
	//		Items.RemoveAt(i);
	//		return true;
	//	}
	//}
	return false;
}

bool UDiaInventoryComponent::MoveItem(const FGuid& InstanceID, int32 NewPosX, int32 NewPosY)
{
	// 기존 위치에서 제거 후 새 위치에 배치
	return false;
}

bool UDiaInventoryComponent::CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY) const
{
	// 기본 범위 검사
	if (PosX < 0 || PosY < 0 || 
		PosX + ItemWidth > GetGridWidth() || 
		PosY + ItemHeight > GetGridHeight())
	{
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

