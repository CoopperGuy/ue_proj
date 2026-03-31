// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DiaDamagePopup.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UDiaDamagePopup::InitializeDamagePopup(float DamageAmount)
{
	if (DamageText)
	{
		FText DamageTextValue = FText::AsNumber(DamageAmount);
		DamageText->SetText(DamageTextValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiaDamagePopup::InitializeDamagePopup - DamageText is null"));
	}
	if (DamagePopupAnimation)
	{
		PlayAnimation(DamagePopupAnimation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiaDamagePopup::InitializeDamagePopup - DamagePopupAnimation is null"));
	}
}
