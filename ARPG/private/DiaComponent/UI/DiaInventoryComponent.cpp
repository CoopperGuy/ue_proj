// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/DiaOptionManagerComponent.h"

#include "GAS/DiaGameplayTags.h"

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

}

bool UDiaInventoryComponent::TryAddItem(const FInventorySlot& ItemData, int32 PosX, int32 PosY, bool bNotify)
{
	if (ItemData.IsEmpty() || !ItemData.ItemInstance.InstanceID.IsValid() ||
		InventoryItems.Contains(ItemData.ItemInstance.InstanceID))
	{
		return false;
	}

	UItemSubsystem* ItemSubsystem = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>()
		: nullptr;
	if (!ItemSubsystem) return false;

	int32 OutPosX = PosX;
	int32 OutPosY = PosY;
	const int32 ItemWidth = ItemSubsystem->GetItemWidth(ItemData.ItemInstance);
	const int32 ItemHeight = ItemSubsystem->GetItemHeight(ItemData.ItemInstance);

	if (PosX == -1 && PosY == -1)
	{
		if (!FindPlaceForItem(ItemWidth, ItemHeight, OutPosX, OutPosY))
			return false;
	}
	else if (!CanPlaceItemAt(ItemWidth, ItemHeight, PosX, PosY))
	{
		return false;
	}

	FInventorySlot NewItem = ItemData;
	NewItem.GridX = OutPosX;
	NewItem.GridY = OutPosY;

	FillGrid(ItemWidth, ItemHeight, OutPosX, OutPosY, NewItem.ItemInstance);
	InventoryItems.Add(NewItem.ItemInstance.InstanceID, NewItem);
	if (bNotify)
	{
		OnItemAdded.Broadcast(NewItem);
	}
	return true;	
}

bool UDiaInventoryComponent::RequestMoveItem(const FGuid& InstanceID, int32 DestX, int32 DestY)
{
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

	const int32 ItemWidth = ItemSubsystem->GetItemWidth(InventoryItem->ItemInstance);
	const int32 ItemHeight = ItemSubsystem->GetItemHeight(InventoryItem->ItemInstance);
	const int32 OldGridX = InventoryItem->GridX;
	const int32 OldGridY = InventoryItem->GridY;
	if (!ClearGrid(ItemWidth, ItemHeight, OldGridX, OldGridY))
	{
		return false;
	}

	if (!CanPlaceItemAt(ItemWidth, ItemHeight, DestX, DestY))
	{
		FillGrid(ItemWidth, ItemHeight, OldGridX, OldGridY, InventoryItem->ItemInstance);
		return false;
	}

	FillGrid(ItemWidth, ItemHeight, DestX, DestY, InventoryItem->ItemInstance);
	InventoryItem->GridX = DestX;
	InventoryItem->GridY = DestY;
	OnItemMoved.Broadcast(InstanceID, DestX, DestY);

#ifdef UE_EDITOR
	UE_LOG(LogARPG, Log, TEXT("Item moved to new position: (%d, %d)"), DestX, DestY);
#endif
	return true;
}

bool UDiaInventoryComponent::RemoveItem(const FGuid& InstanceID, bool bNotify)
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
		const FGuid RemovedID = InstanceID;
		InventoryItems.Remove(RemovedID);
		if (bNotify)
		{
			OnItemRemoved.Broadcast(RemovedID);
		}
		UE_LOG(LogARPG, Log, TEXT("Item successfully removed from inventory: %s"), *RemovedID.ToString());
		return true;
	}
	return false;
}

void UDiaInventoryComponent::FillGrid(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY, const FItemInstance& InstanceInfo)
{
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
	if (PosX < 0 || PosY < 0 || 
		PosX + ItemWidth > GetGridWidth() || 
		PosY + ItemHeight > GetGridHeight())
	{
#ifdef UE_EDITOR
		UE_LOG(LogARPG, Warning, TEXT("Cannot place item at (%d, %d): Out of bounds."), PosX, PosY);
#endif // UE_EDITOR
		return false;
	}

	for (int32 Y = PosY; Y < PosY + ItemHeight; ++Y)
	{
		for (int32 X = PosX; X < PosX + ItemWidth; ++X)
		{
			int32 CellIndex = Y * GetGridWidth() + X;
			if (CellIndex >= 0 && CellIndex < InventoryGrid.Cells.Num())
			{
				if (InventoryGrid.Cells[CellIndex].bOccupied)
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

	return false; 
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
	InventoryItems.Empty();
	InventoryGrid = FInventoryGrid(GridWidth, GridHeight);

	if (!IsValid(SaveGameInstance))
	{
		return;
	}

	UItemSubsystem* ItemSubsystem = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>()
		: nullptr;
	if (!ItemSubsystem) return;

	for (const FInventorySlot& SavedItem : SaveGameInstance->InventorySlots)
	{
		if(CanPlaceItemAt(ItemSubsystem->GetItemWidth(SavedItem.ItemInstance), ItemSubsystem->GetItemHeight(SavedItem.ItemInstance), SavedItem.GridX, SavedItem.GridY))
		{
			InventoryItems.Add(SavedItem.ItemInstance.InstanceID, SavedItem);
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
