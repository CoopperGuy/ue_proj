// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUDWidget.h"
#include "UI/Orb.h"
#include "UI/MonsterHP.h"
#include "UI/Inventory/MainInventory.h"
#include "UI/CharacterStatus/StatusWidget.h"
#include "UI/Inventory/EquipWidget.h"
#include "UI/System/DiaPrimaryLayout.h"
#include "UI/Skill/SkillPanelWidget.h"
#include "UI/SkillQuickSlotWidget.h"
#include "UI/Skill/SkillQuickSlot.h"

#include "GAS/DiaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/DiaCharacter.h"
#include "Controller/DiaController.h"

#include "Monster/DiaMonster.h"

#include "UI/DiaCaution.h"

#include "Blueprint/WidgetTree.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetMonsterHPVisibility(ESlateVisibility::Collapsed);
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	CharacterStatus->SetVisibility(ESlateVisibility::Collapsed);
	EquipmentWidget->SetVisibility(ESlateVisibility::Collapsed);
	SkillPanelWidget->SetVisibility(ESlateVisibility::Collapsed);
	SkillQuickSlotWidget->SetVisibility(ESlateVisibility::Visible);

	ADiaBaseCharacter* OwningActor = Cast<ADiaBaseCharacter>(GetOwningPlayerPawn());
	ADiaController* OwningController = Cast<ADiaController>(GetOwningPlayer());
	if (IsValid(OwningActor))
	{
		UAbilitySystemComponent* AbilitySystem = OwningActor->GetAbilitySystemComponent();
		if (IsValid(AbilitySystem))
		{
			//player는 이런식으로 설정한다 치는데, 몬스터는 어떻게?하지?
			HealthChangedDelegateHandle =
				AbilitySystem
				->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetHealthAttribute())
				.AddUObject(this, &UHUDWidget::HandleHealthChanged);

			ManaChangedDelegateHandle =
				AbilitySystem
				->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetManaAttribute())
				.AddUObject(this, &UHUDWidget::HandleManaChanged);


			//// 초기값 1회 반영
			const float Health = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
			const float MaxHealth = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute());

			const float Mana = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetManaAttribute());
			const float MaxMana = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxManaAttribute());

			UpdateOrbPercentage(OrbType::OT_HP, Health / MaxHealth);
			UpdateOrbPercentage(OrbType::OT_MP, Mana / MaxMana);
		}
	}
	if (IsValid(OwningController))
	{
		OwningController->GetOnTargetChanged().AddUObject(this, &UHUDWidget::UpdateTagetMonster);
	}
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

void UHUDWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	const float NewHealth = Data.NewValue;

	ADiaBaseCharacter* OwningActor = Cast<ADiaBaseCharacter>(GetOwningPlayerPawn());
	UAbilitySystemComponent* AbilitySystem = OwningActor->GetAbilitySystemComponent();

	const float MaxHealth = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute());

	UpdateOrbPercentage(OrbType::OT_HP, NewHealth / MaxHealth);
}

void UHUDWidget::HandleManaChanged(const FOnAttributeChangeData& Data)
{
	const float OldMana = Data.OldValue;
	const float NewMana = Data.NewValue;

	UE_LOG(LogTemp, Warning, TEXT("Mana Changed from %f to %f"), (NewMana), NewMana);

	ADiaBaseCharacter* OwningActor = Cast<ADiaBaseCharacter>(GetOwningPlayerPawn());
	UAbilitySystemComponent* AbilitySystem = OwningActor->GetAbilitySystemComponent();

	const float MaxMana = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxManaAttribute());

	UE_LOG(LogTemp, Warning, TEXT("Mana Changed: %f / %f"), NewMana, MaxMana);
	UpdateOrbPercentage(OrbType::OT_MP, (NewMana) / MaxMana);
}

void UHUDWidget::UpdateTagetMonster(ADiaBaseCharacter* NewTarget)
{
	TargetMonster = NewTarget;
	if (IsValid(TargetMonster))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Target Changed: %s"), *TargetMonster->GetName());
		MonsterHPWidget->SetVisibility(ESlateVisibility::Visible);

		UAbilitySystemComponent* AbilitySystem = TargetMonster->GetAbilitySystemComponent();

		const float CurHP = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
		const float MaxHP = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute());
		UpdateMonsterPercentage(BarType::BT_HP, CurHP / MaxHP);
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("Target Cleared"));
		SetMonsterHPVisibility(ESlateVisibility::Collapsed);
	}


#if defined(WITH_EDITOR) || UE_BUILD_DEVELOPMENT
	if (TargetMonster)
	{
		UE_LOG(LogTemp, Log, TEXT("Target Changed: %s"), *TargetMonster->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Target Cleared"));
	}
#endif

}

void UHUDWidget::RegisteSkillOnQuickSlotWidget(int32 SkillID, int32 SlotIndex)
{
	if (SkillQuickSlotWidget)
	{
		SkillQuickSlotWidget->UpdateSkillSlot(SlotIndex, SkillID);
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

