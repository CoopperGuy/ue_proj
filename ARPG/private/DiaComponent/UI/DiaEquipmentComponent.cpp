// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "DiaComponent/DiaStatComponent.h"

UDiaEquipmentComponent::UDiaEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

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

	//스텟에 적용
	ApplyEquipmentStats(Item, Slot);
}

void UDiaEquipmentComponent::ApplyEquipmentStats(const FEquippedItem& Item, EEquipmentSlot Slot)
{
	TMap<EItemStat, float> Stats = Item.ItemInstance.BaseItem.BaseStats;
	for(const auto& StatPair : Stats)
	{
		EItemStat StatType = StatPair.Key;
		float Value = StatPair.Value;
	}
}
