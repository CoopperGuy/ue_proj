// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/DiaOptionManagerComponent.h"

#include "GAS/DiaGameplayTags.h"

#include "Controller/DiaController.h"

#include "GameMode/DungeonGameMode.h"

#include "UI/HUDWidget.h"
#include "UI/Item/ItemWidget.h"
#include "UI/Inventory/MainInventory.h"
#include "System/ItemSubsystem.h"
#include "System/DiaSaveGame.h"
#include "Utils/InventoryUtils.h"
#include "Logging/ARPGLogChannels.h"

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
	UItemSubsystem* ItemSubsystem = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>()
		: nullptr;
	if (!ItemSubsystem) return false;

	int32 InvenIdx = 0;
	int32 OutPosX = 0;
	int32 OutPosY = 0;
	const int32 ItemWidth = ItemSubsystem->GetItemWidth(ItemData.ItemInstance);
	const int32 ItemHeight = ItemSubsystem->GetItemHeight(ItemData.ItemInstance);

	//자동으로 찾는 함수 호출
	if (!FInventoryUtils::FindPlaceForItem(this, ItemWidth, ItemHeight, OutPosX, OutPosY)) 
		return false;

	FInventorySlot NewItem = ItemData;
	NewItem.GridX = OutPosX;
	NewItem.GridY = OutPosY;

	bool res = InvenWidget->AddItem(NewItem, nullptr, OutPosY, OutPosX);

	if (res)
	{
		FillGrid(ItemWidth, ItemHeight, OutPosX, OutPosY, NewItem.ItemInstance);
		InventoryItems.Add(NewItem.ItemInstance.InstanceID, NewItem);
	}

	return res;
}

bool UDiaInventoryComponent::RequestMoveItem(const FGuid& InstanceID, int32 DestX, int32 DestY, UMainInventory* InventoryWidget)
{
	// const FInventorySlot* ItemInfo = InventoryWidget->GetItemDataAtGuid(InstanceID);
	FInventorySlot* InventoryItem = InventoryItems.Find(InstanceID);
	if (!InventoryItem)
	{
#ifdef UE_EDITOR
		UE_LOG(LogARPG, Warning, TEXT("MoveItem: Item with InstanceID %s not found."), *InstanceID.ToString());
#endif // UE_EDITOR
		return false;
	}

	UItemSubsystem* ItemSubsystem = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>()
		: nullptr;
	if (!ItemSubsystem) return false;

	
	if (UItemWidget* ItemWidget = InventoryWidget->GetItemWidgetAtGuid(InstanceID))
	{
		const int32 ItemWidth = ItemSubsystem->GetItemWidth(InventoryItem->ItemInstance);
		const int32 ItemHeight = ItemSubsystem->GetItemHeight(InventoryItem->ItemInstance);
		
		if (!ClearGrid(ItemWidth, ItemHeight, InventoryItem->GridX, InventoryItem->GridY))
		{
			ItemWidget->SetRenderOpacity(1.0f);
			return false;
		}

		FillGrid(ItemWidth, ItemHeight, DestX, DestY, InventoryItem->ItemInstance);
		InventoryItem->GridX = DestX;
		InventoryItem->GridY = DestY;

		InventoryWidget->MoveContainItem(InstanceID, DestX, DestY);
		ItemWidget->SetRenderOpacity(1.0f);
		InventoryWidget->UpdateItemPosition(ItemWidget, DestX, DestY);

#ifdef UE_EDITOR
		UE_LOG(LogARPG, Log, TEXT("Item moved to new position: (%d, %d)"), DestX, DestY);
#endif
		return true;
	}

	return false;
}

bool UDiaInventoryComponent::RemoveItem(const FGuid& InstanceID, UMainInventory* InvenWidget)
{
	// const FInventorySlot* ItemInfo = InvenWidget->GetItemDataAtGuid(InstanceID);
	const FInventorySlot* InventoryItem = InventoryItems.Find(InstanceID);
	if (!InventoryItem) return false;
	UItemSubsystem* ItemSubsystem = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>()
		: nullptr;
	if (!ItemSubsystem) return false;

	int32 ItemWidth = ItemSubsystem->GetItemWidth(InventoryItem->ItemInstance);
	int32 ItemHeight = ItemSubsystem->GetItemHeight(InventoryItem->ItemInstance);
	int32 PosX = InventoryItem->GridX;
	int32 PosY = InventoryItem->GridY;

	if (ClearGrid(ItemWidth, ItemHeight, PosX, PosY))
	{
		if (InvenWidget->RemoveContainItem(InstanceID))
		{
			InventoryItems.Remove(InstanceID);
			UE_LOG(LogARPG, Log, TEXT("Item successfully removed from inventory: %s"), *InstanceID.ToString());
			return true;
		}
	}
	return false;
}

void UDiaInventoryComponent::FillGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY, const FItemInstance& InstanceInfo)
{
	// 아이템이 차지하는 그리드 공간을 채움
	for (int32 y = PosY; y < PosY + ItemHeight; ++y)
	{
		for (int32 x = PosX; x < PosX + ItemWidth; ++x)
		{
			InventoryGrid[y * GridWidth + x].bOccupied = true;
			InventoryGrid[y * GridWidth + x].ItemInstanceID = InstanceInfo.InstanceID;
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
		UE_LOG(LogARPG, Warning, TEXT("Cannot place item at (%d, %d): Out of bounds."), PosX, PosY);
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
				if (InventoryGrid.Cells[CellIndex].bOccupied) // 셀이 이미 점유됨
				{
#ifdef UE_EDITOR
					UE_LOG(LogARPG, Warning, TEXT("Cannot place item at (%d, %d) : Cell(% d, % d) is already occupied."), PosX, PosY, X, Y);
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

void UDiaInventoryComponent::SaveInventoryToSaveGame(UDiaSaveGame* SaveGameInstance) const
{
	for (const auto& ItemPair : InventoryItems)
	{
		const FInventorySlot& ItemData = ItemPair.Value;
		SaveGameInstance->InventorySlots.Add(ItemData);
	}
}

void UDiaInventoryComponent::LoadInventoryFromSaveGame(const UDiaSaveGame* SaveGameInstance)
{
	UItemSubsystem* ItemSubsystem = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>()
		: nullptr;
	if (!ItemSubsystem) return;

	for (const FInventorySlot& SavedItem : SaveGameInstance->InventorySlots)
	{
		InventoryItems.Add(SavedItem.ItemInstance.InstanceID, SavedItem);
		if(CanPlaceItemAt(ItemSubsystem->GetItemWidth(SavedItem.ItemInstance), ItemSubsystem->GetItemHeight(SavedItem.ItemInstance), SavedItem.GridX, SavedItem.GridY))
		{
			FillGrid(ItemSubsystem->GetItemWidth(SavedItem.ItemInstance), ItemSubsystem->GetItemHeight(SavedItem.ItemInstance), SavedItem.GridX, SavedItem.GridY, SavedItem.ItemInstance);
		}
	}
}

void UDiaInventoryComponent::AddGoldInventoryWithCheckOption(int32 Amount, UDiaOptionManagerComponent* OptionManager)
{
	const float GoldFind = IsValid(OptionManager)
		? OptionManager->GetTotalOptionMagnitudeByTag(FDiaGameplayTags::Get().ItemOptionGoldFind)
		: 0.0f;

	const int32 FinalAmount = FMath::RoundToInt(Amount * (1.0f + GoldFind * 0.01f));
	AddGold(FinalAmount);
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
				InventoryGrid.Cells[CellIndex].bOccupied = false;
				InventoryGrid.Cells[CellIndex].ItemInstanceID.Invalidate();
			}
			else
			{
#ifdef UE_EDITOR
				UE_LOG(LogARPG, Warning, TEXT("ClearGrid: CellIndex %d outof bounds."), CellIndex);
#endif // UE_EDITOR
					return false;
			}
		}
	}

	return true;
}

void UDiaInventoryComponent::SetGold(int32 NewGold)
{
	NewGold = FMath::Max(0, NewGold);

	const int32 OldGold = Gold;
	if (OldGold == NewGold)
	{
		return;
	}

	Gold = NewGold;

	const int32 Delta = Gold - OldGold;

	OnGoldChanged.Broadcast(Gold, Delta);
}

void UDiaInventoryComponent::AddGold(int32 Amount)
{
	if (Amount == 0)
	{
		return;
	}
	SetGold(Gold + Amount);
}

const FInventorySlot* UDiaInventoryComponent::GetItemDataAtGuid(const FGuid& InstanceID) const
{
	const FInventorySlot* InvenItem = InventoryItems.Find(InstanceID);
	if (InvenItem)
	{
		return InvenItem;
	}

	return nullptr;
}
