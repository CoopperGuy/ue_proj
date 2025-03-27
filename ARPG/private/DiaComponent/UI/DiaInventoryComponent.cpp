// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaInventoryComponent.h"


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

bool UDiaInventoryComponent::TryAddItem(const FInventoryItem& ItemData, int32 PosX, int32 PosY)
{
	return false;
}

bool UDiaInventoryComponent::CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
	return false;
}

