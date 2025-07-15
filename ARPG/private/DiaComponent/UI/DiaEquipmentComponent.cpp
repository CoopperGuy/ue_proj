// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaEquipmentComponent.h"


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

