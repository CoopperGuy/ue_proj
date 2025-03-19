// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

enum class OrbType
{
	OT_HP,
	OT_MP,
	OT_MAX
};
class UOrb;
/**
 * 
 */
UCLASS()
class ARPG_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	
	void UpdateOrbPercentage(OrbType _Type, float _Percentage);
protected:
	UPROPERTY(meta = (BindWidget))
	UOrb* HpWidget;

	
	UPROPERTY(meta = (BindWidget))
	UOrb* MpWidget;
};
