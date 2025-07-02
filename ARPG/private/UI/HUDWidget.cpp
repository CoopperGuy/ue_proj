// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUDWidget.h"
#include "UI/Orb.h"
#include "UI/MonsterHP.h"
#include "UI/Inventory/MainInventory.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetMonsterHPVisibility(ESlateVisibility::Collapsed);
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UHUDWidget::UpdateOrbPercentage(OrbType _Type, float _Percentage)
{
	switch (_Type)
	{
	case OrbType::OT_HP:
		HpWidget->UpdatePercentage(_Percentage);
		break;
	case OrbType::OT_MP:
		MpWidget->UpdatePercentage(_Percentage);
		break;
	}
}

//-> DiaCombatComponent 
void UHUDWidget::UpdateMonsterPercentage(BarType _Type, float _Percentage)
{
	MonsterHPWidget->SetVisibility(ESlateVisibility::Visible);
	switch (_Type)
	{
	case BarType::BT_HP:
		MonsterHPWidget->UpdatePercentage(_Percentage);
		break;
	}
}

void UHUDWidget::SetMonsterHPVisibility(ESlateVisibility _Visibility)
{
	MonsterHPWidget->SetVisibility(_Visibility);
}
