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

enum class BarType
{
	BT_HP,
	BT_MAX
};

class UOrb;
class UMonsterHP;
class UMainInventory;
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
	void UpdateMonsterPercentage(BarType _Type, float _Percentage);

protected:
	UPROPERTY(meta = (BindWidget))
	UOrb* HpWidget;

	UPROPERTY(meta = (BindWidget))
	UOrb* MpWidget;

	UPROPERTY(meta = (BindWidget))
	UMonsterHP* MonsterHPWidget;

	UPROPERTY(meta = (BindWidget))
	UMainInventory* InventoryWidget;
public:
	void SetMonsterHPVisibility(ESlateVisibility _Visibility);
	FORCEINLINE UMainInventory* GetInventoryWidget() const { return InventoryWidget; }
};
