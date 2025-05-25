// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "GameMode/DungeonGameMode.h"

#include "UI/HUDWidget.h"
#include "UI/Item/ItemWidget.h"
#include "UI/Inventory/MainInventory.h"

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
	if (!FindPlaceItem(ItemData, OutPosX, OutPosY)) return false;

	InvenIdx = OutPosX * GridWidth + OutPosY;
	//ui작업
	bool res = InvenWidget->AddItemToInventory(ItemData, ItemData.GridX, ItemData.GridY, OutPosX, OutPosY);
	//삽입에 성공하면, 여기서도 아이템이 삽입 되었다는 사실을 추가한다.
	if (res)
	{
		Items.Add(ItemData);
		FillGrid(ItemData.GridX, ItemData.GridY, OutPosX, OutPosY);
		
		return true;
	}
		
	return false;
}

bool UDiaInventoryComponent::FindPlaceItem(const FInventoryItem& ItemData, int32& OutPosX, int32& OutPosY)
{
	//아이템이 들어갈 위치를 탐색한다.
	//다 찾으면 treu 리턴 해야함.
	for (int32 y = 0; y < GridHeight - ItemData.GridY; ++y)
	{
		for (int32 x = 0; x < GridWidth - ItemData.GridX; ++x)
		{
			//만약 아이템이 들어갈 수 있는 공간이라면 true
			//체크 로직 좌상단의 좌표를 반환한다.
			if (CanPlaceItemAt(ItemData.GridX, ItemData.GridY, y, x))
			{
				OutPosX = y;
				OutPosY = x;
				return true;
			}
		}
	}
	//하나도 못찾으면 false
	return false;
}

bool UDiaInventoryComponent::CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
	// 아이템 크기에 따른 그리드 공간 확인
	for (int32 y = PosY; y < PosY + ItemHeight; ++y)
	{
		for (int32 x = PosX; x < PosX + ItemWidth; ++x)
		{
			if (x >= GridWidth || y >= GridHeight || InventoryGrid[y * GridWidth + x])
				return false;
		}
	}
	return true;
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
	//		ClearGrid(Items[i].GridX, Items[i].GridY, ItemWidth, ItemHeight);
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

