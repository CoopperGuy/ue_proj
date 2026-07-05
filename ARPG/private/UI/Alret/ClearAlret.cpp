// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Alret/ClearAlret.h"
#include "Animation/WidgetAnimation.h"
#include "Logging/ARPGLogChannels.h"

void UClearAlret::NativeOnActivated()
{
	Super::NativeOnActivated();

	if (Anim_FadeOut)
	{
		PlayAnimation(Anim_FadeOut);
	}
	else
	{
		UE_LOG(LogARPG, Warning, TEXT("UClearAlret::NativeOnActivated - Anim_FadeOut is null"));
	}

	GetWorld()->GetTimerManager().SetTimer(DeactivateTimerHandle, this, &UClearAlret::HandleDeactivate, DisplayDuration, false);
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
