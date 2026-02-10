// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "DiaComponent/DiaStatComponent.h"
#include "Controller/DiaController.h"
#include "DiaBaseCharacter.h"
#include "AbilitySystemComponent.h"

#include "GAS/Effects/DiaGE_StatApply.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"

#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"

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

void UDiaEquipmentComponent::EquipItem(const FEquippedItem& Item, EEquipmentSlot Slot)
{
	//아이템 추가
	EquipmentMap.Add(Slot, Item);
	UE_LOG(LogTemp, Log, TEXT("EquipItem: Slot %s에 아이템 장착됨."), *UEnum::GetValueAsString(Slot));
}

void UDiaEquipmentComponent::UnEquipItem(EEquipmentSlot Slot)
{
	//아이템 제거
	if (FEquippedItem* EquippedItem = EquipmentMap.Find(Slot))
	{
		//스텟에서 제거
		OnItemUnEquipped.Broadcast(Slot);
		UE_LOG(LogTemp, Log, TEXT("UnEquipItem: Slot %s에서 아이템 제거됨."), *UEnum::GetValueAsString(Slot));
	}
}

void UDiaEquipmentComponent::UnEquipItemFinish(EEquipmentSlot Slot)
{
	if (FEquippedItem* EquippedItem = EquipmentMap.Find(Slot))
	{
		//스텟에서 제거
		EquipmentMap.Remove(Slot);
		UE_LOG(LogTemp, Log, TEXT("UnEquipItem: Slot %s에서 아이템 제거됨."), *UEnum::GetValueAsString(Slot));
	}

}

const FEquippedItem* UDiaEquipmentComponent::GetEquippedItem(EEquipmentSlot Slot) const
{
	if (const FEquippedItem* EquippedItem = EquipmentMap.Find(Slot))
	{
		return EquippedItem;
	}
	return nullptr;
}
