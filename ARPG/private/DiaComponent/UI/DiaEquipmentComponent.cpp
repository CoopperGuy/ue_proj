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
		if (StatComponent.IsValid())
		{
			// StatComponent에 스탯 적용
			//추가 스탯으로 처리한다.
			StatComponent->SetAdditionalStat(StatType, Value);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("StatComponent is not valid! Cannot apply stats for slot: %s"), *UEnum::GetValueAsString(Slot));
		}
	}
}

void UDiaEquipmentComponent::SetStatComponent(UDiaStatComponent* InStatComponent)
{
	if (InStatComponent)
	{
		StatComponent = InStatComponent;
	}
	else
	{
		StatComponent.Reset();
	}
}
