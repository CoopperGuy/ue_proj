// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Alret/ClearAlret.h"

void UClearAlret::NativeOnActivated()
{
	Super::NativeOnActivated();

	GetWorld()->GetTimerManager().SetTimer(DeactivateTimerHandle, this, &UClearAlret::HandleDeactivate, 3.0f, false);
}

void UClearAlret::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DeactivateTimerHandle);
	}

	RemoveFromParent();
}

void UClearAlret::HandleDeactivate()
{
	DeactivateWidget();
}
