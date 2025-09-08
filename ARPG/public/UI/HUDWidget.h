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
class UStatusWidget;
class UEquipWidget;
class UDiaPrimaryLayout;
class USkillPanelWidget;
//class USkillQuickSlotPannelWidget;
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

	void CreateCautionWidget();
protected:
	UPROPERTY(meta = (BindWidget))
	UOrb* HpWidget;

	UPROPERTY(meta = (BindWidget))
	UOrb* MpWidget;

	UPROPERTY(meta = (BindWidget))
	UMonsterHP* MonsterHPWidget;

	UPROPERTY(meta = (BindWidget))
	UMainInventory* InventoryWidget;

	UPROPERTY(meta = (BindWidget))
	UStatusWidget* CharacterStatus;

	UPROPERTY(meta = (BindWidget))
	UEquipWidget* EquipmentWidget;

	UPROPERTY(meta = (BindWidget))
	USkillPanelWidget* SkillPanelWidget;

	//UPROPERTY(meta = (BindWidget))
	//USkillQuickSlotPannelWidget* SkillQuickSlotWidget;

	UPROPERTY()
	UDiaPrimaryLayout* DiaPrimaryLayout;
public:
	void SetMonsterHPVisibility(ESlateVisibility _Visibility);
	FORCEINLINE UMainInventory* GetInventoryWidget() const { return InventoryWidget; }
	FORCEINLINE UStatusWidget* GetCharacterStatusWidget() const { return CharacterStatus; }
	FORCEINLINE UEquipWidget* GetEquipmentWidget() const { return EquipmentWidget; }

	UUserWidget* FindWidgetByName(const FName& WidgetName);
};

