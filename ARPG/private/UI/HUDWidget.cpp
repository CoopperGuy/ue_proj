// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUDWidget.h"
#include "UI/Orb.h"
#include "UI/MonsterHP.h"
#include "UI/Inventory/MainInventory.h"
#include "UI/CharacterStatus/StatusWidget.h"
#include "UI/Inventory/EquipWidget.h"
#include "UI/System/DiaPrimaryLayout.h"

#include "UI/DiaCaution.h"

#include "Blueprint/WidgetTree.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetMonsterHPVisibility(ESlateVisibility::Collapsed);
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	CharacterStatus->SetVisibility(ESlateVisibility::Collapsed);
	EquipmentWidget->SetVisibility(ESlateVisibility::Collapsed);
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

UUserWidget* UHUDWidget::FindWidgetByName(const FName& WidgetName)
{
	if (!WidgetTree)
	{
		UE_LOG(LogTemp, Warning, TEXT("WidgetTree is null in UHUDWidget::FindWidgetByName"));
		return nullptr;
	}
	UWidget* FoundWidget = WidgetTree->FindWidget(WidgetName);
	if (FoundWidget)
	{
		return Cast<UUserWidget>(FoundWidget);
	}
	return nullptr;
}

