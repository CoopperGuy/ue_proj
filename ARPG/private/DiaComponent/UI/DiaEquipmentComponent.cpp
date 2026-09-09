// Fill out your copyright notice in the Description page of Project Settings.

#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/DiaOptionManagerComponent.h"

#include "System/DiaSaveGame.h"

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
	// 장비를 장착 했다는 정보 저장
	FEquippedItem Item = FEquippedItem::FromInventorySlot(InventorySlot);


	if (!InventoryComponent->RemoveItem(InventorySlot.ItemInstance.InstanceID))
	{
		return false;
	}

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

	return true;
}

bool UDiaEquipmentComponent::TryUnEquipToInventory(EEquipmentSlot SlotType, FInventorySlot& OutInventorySlot)
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


	if (!InventoryComponent->TryAddItem(InventorySlot))
	{
		return false;
	}

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

	OnItemUnEquipped.Broadcast(SlotType);

	return true;
}
