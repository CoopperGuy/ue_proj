// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/DiaItemWidgetComponent.h"

UDiaItemWidgetComponent::UDiaItemWidgetComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReceiveHardwareInput = true;
}

void UDiaItemWidgetComponent::EnsureReceiveHardwareInput()
{
	bReceiveHardwareInput = true;
}
