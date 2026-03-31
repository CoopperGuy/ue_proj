// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiaDamagePopup.generated.h"

class UTextBlock;
class UWidgetAnimation;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaDamagePopup : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeDamagePopup(float DamageAmount);
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DamageText;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* DamagePopupAnimation;
};
