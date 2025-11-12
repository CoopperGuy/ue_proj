// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaLevelComponent.h"


UDiaLevelComponent::UDiaLevelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UDiaLevelComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UDiaLevelComponent::LevelUp()
{
	Level++;
	OnLevelUpDelegate.Broadcast(Level);
}

