// Fill out your copyright notice in the Description page of Project Settings.

#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/DiaOptionManagerComponent.h"

#include "System/DiaSaveGame.h"
#include "System/ItemSubsystem.h"

#include "Logging/ARPGLogChannels.h"


UDiaEquipmentComponent::UDiaEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDiaEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDiaEquipmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDiaEquipmentComponent::EquipItem(const FEquippedItem& Item, EEquipmentSlot SlotType)
{
	EquipmentMap.Add(SlotType, Item);
}

void UDiaEquipmentComponent::UnEquipItem(EEquipmentSlot SlotType)
{
	if (FEquippedItem* EquippedItem = EquipmentMap.Find(SlotType))
	{
		return;
	}
}

void UDiaEquipmentComponent::UnEquipItemFinish(EEquipmentSlot SlotType)
{
	if (FEquippedItem* EquippedItem = EquipmentMap.Find(SlotType))
	{
		AddDebugEvent(FString::Printf(TEXT("Unequipped %s from %s"),
			*EquippedItem->ItemInstance.ItemID.ToString(),
			*UEnum::GetValueAsString(SlotType)), true);

		EquipmentMap.Remove(SlotType);
		UE_LOG(LogARPG, Log, TEXT("UnEquipItem: Slot %s unequipped item."), *UEnum::GetValueAsString(SlotType));
	}
}

const FEquippedItem* UDiaEquipmentComponent::GetEquippedItem(EEquipmentSlot SlotType) const
{
	if (const FEquippedItem* EquippedItem = EquipmentMap.Find(SlotType))
	{
		return EquippedItem;
	}

	return nullptr;
}

void UDiaEquipmentComponent::AddDebugEvent(const FString& Message, bool bPassed)
{
	FEquipmentDebugEvent Event;
	Event.Sequence = ++DebugEventSequence;
	Event.bPassed = bPassed;
	Event.Message = Message;

	DebugEvents.Add(Event);
	constexpr int32 MaxDebugEvents = 20;
	if (DebugEvents.Num() > MaxDebugEvents)
	{
		DebugEvents.RemoveAt(0, DebugEvents.Num() - MaxDebugEvents);
	}
}

void UDiaEquipmentComponent::SaveEquipmentToSaveGame(UDiaSaveGame* SaveGameInstance) const
{
	for(const auto& EquipmentPair : EquipmentMap)
	{
		const EEquipmentSlot SlotType = EquipmentPair.Key;
		const FEquippedItem& EquippedItem = EquipmentPair.Value;
		SaveGameInstance->EquippedItems.Add(SlotType, EquippedItem);
	}
}

void UDiaEquipmentComponent::LoadEquipmentFromSaveGame(const UDiaSaveGame* SaveGameInstance)
{
	EquipmentMap.Empty();

	if (!IsValid(SaveGameInstance))
	{
		return;
	}

	for(const auto& SaveData : SaveGameInstance->EquippedItems)
	{
		EquipmentMap.Add(SaveData.Key, SaveData.Value);
	}
}

bool UDiaEquipmentComponent::TryEquipFromInventory(const FInventorySlot& InventorySlot, EEquipmentSlot SlotType)
{
	if (!IsValid(InventoryComponent) || !IsValid(OptionManagerComponent))
	{
		return false;
	}

	UItemSubsystem* ItemSubsystem = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
	const FInventorySlot* StoredItem = InventoryComponent->GetItemDataAtGuid(InventorySlot.ItemInstance.InstanceID);
	if (!IsValid(ItemSubsystem) || !StoredItem || StoredItem->IsEmpty())
	{
		return false;
	}

	// Use component data; a UI copy can be stale or cleared by a notification.
	const FInventorySlot SourceItem = *StoredItem;
	const EEquipmentSlot ItemSlot = ItemSubsystem->GetEquipmentSlot(SourceItem.ItemInstance);
	const bool bRingItem = ItemSlot == EEquipmentSlot::EES_RingL || ItemSlot == EEquipmentSlot::EES_RingR;
	const bool bRingTarget = SlotType == EEquipmentSlot::EES_RingL || SlotType == EEquipmentSlot::EES_RingR;
	if (SlotType == EEquipmentSlot::EES_None || SlotType >= EEquipmentSlot::EES_Max ||
		(ItemSlot != SlotType && !(bRingItem && bRingTarget)))
	{
		return false;
	}

	const bool bReplacingItem = GetEquippedItem(SlotType) != nullptr;
	if (!InventoryComponent->RemoveItem(SourceItem.ItemInstance.InstanceID, false))
	{
		return false;
	}

	FInventorySlot ReturnedItem;
	if (bReplacingItem && !TryUnEquipToInventory(SlotType, ReturnedItem, -1, -1, false))
	{
		// No notification or equipment change has occurred. Restore the exact vacated cells.
		const bool bRestored = InventoryComponent->TryAddItem(SourceItem, SourceItem.GridX, SourceItem.GridY, false);
		ensureMsgf(bRestored, TEXT("Failed to restore inventory item after rejected equipment swap."));
		return false;
	}

	const FEquippedItem Item = FEquippedItem::FromInventorySlot(SourceItem);
	EquipItem(Item, SlotType);
	OptionManagerComponent->ApplyEquipmentStats(Item, SlotType);

	for (const auto& option : Item.ItemInstance.PrefixOptions)
	{
		OptionManagerComponent->AddOption(option);
	}
	for (const auto& option : Item.ItemInstance.SuffixOptions)
	{
		OptionManagerComponent->AddOption(option);
	}
	OptionManagerComponent->ApplyEquipmentSlotOption(Item);

	// Publish only the final state; replacement needs no intermediate empty-slot notification.
	InventoryComponent->OnItemRemoved.Broadcast(SourceItem.ItemInstance.InstanceID);
	if (bReplacingItem)
	{
		InventoryComponent->OnItemAdded.Broadcast(ReturnedItem);
	}
	OnItemEquipped.Broadcast(Item, SlotType);
	return true;
}

bool UDiaEquipmentComponent::TryUnEquipToInventory(EEquipmentSlot SlotType, FInventorySlot& OutInventorySlot, int32 PosX, int32 PosY, bool bNotify)
{
	if (!IsValid(InventoryComponent) || !IsValid(OptionManagerComponent))
	{
		return false;
	}

	const FEquippedItem* FoundItem = GetEquippedItem(SlotType);
	if (!FoundItem)
	{
		return false;
	}

	const FEquippedItem Item = *FoundItem;
	FInventorySlot InventorySlot = Item.ToInventorySlot();
	if (!InventoryComponent->TryAddItem(InventorySlot, PosX, PosY, false))
	{
		return false;
	}
	InventorySlot = *InventoryComponent->GetItemDataAtGuid(Item.ItemInstance.InstanceID);

	UnEquipItem(SlotType);
	OptionManagerComponent->RemoveEqipmentStats(Item, SlotType);
	for (const auto& option : Item.ItemInstance.PrefixOptions)
	{
		OptionManagerComponent->RemoveOption(option);
	}
	for (const auto& option : Item.ItemInstance.SuffixOptions)
	{
		OptionManagerComponent->RemoveOption(option);
	}
	UnEquipItemFinish(SlotType);
	OutInventorySlot = InventorySlot;

	if (bNotify)
	{
		InventoryComponent->OnItemAdded.Broadcast(InventorySlot);
		OnItemUnEquipped.Broadcast(SlotType);
	}
	return true;
}
