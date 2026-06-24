// Fill out your copyright notice in the Description page of Project Settings.

#include "DiaComponent/UI/DiaEquipmentComponent.h"

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
	AddDebugEvent(FString::Printf(TEXT("Equipped %s to %s"),
		*Item.ItemInstance.ItemID.ToString(),
		*UEnum::GetValueAsString(SlotType)), true);

	UE_LOG(LogARPG, Log, TEXT("EquipItem: Slot %s equipped item."), *UEnum::GetValueAsString(SlotType));
}

void UDiaEquipmentComponent::UnEquipItem(EEquipmentSlot SlotType)
{
	if (FEquippedItem* EquippedItem = EquipmentMap.Find(SlotType))
	{
		AddDebugEvent(FString::Printf(TEXT("Unequip requested for %s from %s"),
			*EquippedItem->ItemInstance.ItemID.ToString(),
			*UEnum::GetValueAsString(SlotType)), true);

		OnItemUnEquipped.Broadcast(SlotType);
		UE_LOG(LogARPG, Log, TEXT("UnEquipItem: Slot %s unequip requested."), *UEnum::GetValueAsString(SlotType));
		return;
	}

	AddDebugEvent(FString::Printf(TEXT("Unequip requested for empty %s"),
		*UEnum::GetValueAsString(SlotType)), false);
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
	for(const auto& SaveData : SaveGameInstance->EquippedItems)
	{
		EquipmentMap.Add(SaveData.Key, SaveData.Value);
	}
}
