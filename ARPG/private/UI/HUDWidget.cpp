// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUDWidget.h"
#include "UI/Orb.h"
#include "UI/MonsterHP.h"
#include "UI/System/DiaPrimaryLayout.h"
#include "UI/SkillQuickSlotWidget.h"
#include "UI/Skill/SkillQuickSlot.h"
#include "UI/Alret/ClearAlret.h"
#include "UI/DiaCaution.h"
#include "UI/DiaDamagePopup.h"
#include "UI/Rewards/RewardChoicePanel.h"

#include "GAS/DiaGameplayTags.h"
#include "GAS/DiaAttributeSet.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/DiaCharacter.h"
#include "Controller/DiaController.h"

#include "Monster/DiaMonster.h"

#include "DiaGameState.h"

#include "System/GameViewPort/DiaCustomGameViewPort.h"

#include "Blueprint/WidgetTree.h"
#include "Logging/ARPGLogChannels.h"


void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();


	if (!IsValid(SkillQuickSlotWidget) || !IsValid(BossMonsterHPWidget) || !IsValid(MonsterHPWidget) || !IsValid(RewardChoicePanel))
		return;
	if (!IsValid(HpWidget) || !IsValid(MpWidget))
		return;

	SkillQuickSlotWidget->SetVisibility(ESlateVisibility::Visible);
	BossMonsterHPWidget->SetVisibility(ESlateVisibility::Collapsed);
	MonsterHPWidget->SetVisibility(ESlateVisibility::Collapsed);
	RewardChoicePanel->SetVisibility(ESlateVisibility::Collapsed);

	ADiaBaseCharacter* OwningActor = Cast<ADiaBaseCharacter>(GetOwningPlayerPawn());

	// 루트가 Visible이면 화면 전체가 히트 테스트를 먹어서 월드 UWidgetComponent(아이템 이름 등)로 입력이 가지 않음.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetMonsterHPVisibility(ESlateVisibility::Collapsed);

	if (IsValid(OwningActor))
	{
		UnBindCharacterAttributeChangeDelegates();

		CachedAbilitySystem = OwningActor->GetAbilitySystemComponent();
		CachedOwningController = Cast<ADiaController>(GetOwningPlayer());

		BindCharacterAttributeChangeDelegates();
		//// 초기값 1회 반영
		const float Health = CachedAbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
		const float MaxHealth = CachedAbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute());

		const float Mana = CachedAbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetManaAttribute());
		const float MaxMana = CachedAbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxManaAttribute());

		UpdateOrbPercentage(OrbType::OT_HP, Health / MaxHealth);
		UpdateOrbPercentage(OrbType::OT_MP, Mana / MaxMana);

		if (ADiaCharacter* DiaCharacter = Cast<ADiaCharacter>(OwningActor))
		{
			DiaCharacter->RegisteCurrentSkillList();
		}

		if (CachedOwningController->IsValidLowLevel())
		{
			TargetChangedDelegateHandle = CachedOwningController->GetOnTargetChanged().AddUObject(this, &UHUDWidget::UpdateTagetMonster);
		}
	}

	UDiaCustomGameViewPort* DiaCumstomGameViewPort = GetWorld() ? Cast<UDiaCustomGameViewPort>(GetWorld()->GetGameViewport()) : nullptr;
	if (IsValid(DiaCumstomGameViewPort))
	{
		DiaPrimaryLayout = DiaCumstomGameViewPort->GetDiaPrimaryLayout();
	}

	if (IsValid(RewardChoicePanel))
	{
		RewardChoicePanel->OnRewardCardSelected.AddUniqueDynamic(this, &UHUDWidget::OnRewardCardSelected);
	}

}

void UHUDWidget::NativeDestruct()
{
	UnBindCharacterAttributeChangeDelegates();

	if (ADiaController* Controller = CachedOwningController.Get();
		IsValid(Controller) && TargetChangedDelegateHandle.IsValid())
	{
		Controller->GetOnTargetChanged().Remove(TargetChangedDelegateHandle);
		TargetChangedDelegateHandle.Reset();
	}

	if (IsValid(RewardChoicePanel))
	{
		RewardChoicePanel->OnRewardCardSelected.RemoveDynamic(this, &ThisClass::OnRewardCardSelected);
	}

	Super::NativeDestruct();
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
void UHUDWidget::UpdateMonsterPercentage(BarType _Type, float _Percentage, bool IsBoss)
{
	switch (_Type)
	{
	case BarType::BT_HP:
		if (IsBoss)
		{
			MonsterHPWidget->SetVisibility(ESlateVisibility::Collapsed);
			BossMonsterHPWidget->SetVisibility(ESlateVisibility::Visible);

			BossMonsterHPWidget->UpdatePercentage(_Percentage);
		}
		else
		{
			MonsterHPWidget->SetVisibility(ESlateVisibility::Visible);
			BossMonsterHPWidget->SetVisibility(ESlateVisibility::Collapsed);

			MonsterHPWidget->UpdatePercentage(_Percentage);
		}
		break;
	}
}

void UHUDWidget::UpdateExpPercentage(float _Percentage)
{
	SkillQuickSlotWidget->SetExpBarPercent(_Percentage);
}

void UHUDWidget::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	const float NewHealth = Data.NewValue;

	ADiaBaseCharacter* OwningActor = Cast<ADiaBaseCharacter>(GetOwningPlayerPawn());
	UAbilitySystemComponent* AbilitySystem = OwningActor->GetAbilitySystemComponent();
	if (!AbilitySystem)
	{
		return;
	}

	const float MaxHealth = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute());
	UpdateOrbPercentage(OrbType::OT_HP, NewHealth / MaxHealth);
}

void UHUDWidget::HandleManaChanged(const FOnAttributeChangeData& Data)
{
	const float OldMana = Data.OldValue;
	const float NewMana = Data.NewValue;

	//UE_LOG(LogARPG, Warning, TEXT("Mana Changed from %f to %f"), (NewMana), NewMana);

	ADiaBaseCharacter* OwningActor = Cast<ADiaBaseCharacter>(GetOwningPlayerPawn());
	if(!IsValid(OwningActor))
	{
		return;
	}
	UAbilitySystemComponent* AbilitySystem = OwningActor->GetAbilitySystemComponent();
	if(!IsValid(AbilitySystem))
	{
		return;
	}
	const float MaxMana = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxManaAttribute());

	//UE_LOG(LogARPG, Warning, TEXT("Mana Changed: %f / %f"), NewMana, MaxMana);
	UpdateOrbPercentage(OrbType::OT_MP, (NewMana) / MaxMana);
}

void UHUDWidget::HandlExpChanged(const FOnAttributeChangeData& Data)
{
	const float NewExp = Data.NewValue;
	ADiaBaseCharacter* OwningActor = Cast<ADiaBaseCharacter>(GetOwningPlayerPawn());
	if(!IsValid(OwningActor))
	{
		return;
	}
	UAbilitySystemComponent* AbilitySystem = OwningActor->GetAbilitySystemComponent();
	if(!IsValid(AbilitySystem))
	{
		return;
	}
	const float MaxExp = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxExpAttribute());
	//UE_LOG(LogARPG, Warning, TEXT("Exp Changed: %f / %f"), NewExp, MaxExp);

	UpdateExpPercentage(NewExp / MaxExp);
}

void UHUDWidget::UpdateTagetMonster(ADiaBaseCharacter* NewTarget)
{
	TargetMonster = NewTarget;
	if (IsValid(TargetMonster))
	{
		bool IsBoss = TargetMonster->CheckBossMonsterByAbilityTag();
		UE_LOG(LogARPG, Warning, TEXT("Target Changed: %s, IsBoss: %s"), *TargetMonster->GetName(), IsBoss ? TEXT("True") : TEXT("False"));
		UAbilitySystemComponent* AbilitySystem = TargetMonster->GetAbilitySystemComponent();

		const float CurHP = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
		const float MaxHP = AbilitySystem->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute());

		UpdateMonsterPercentage(BarType::BT_HP, CurHP / MaxHP, IsBoss);
		if (CurHP == 0.f)
		{
			SetMonsterHPVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		//UE_LOG(LogARPG, Warning, TEXT("Target Cleared"));
		SetMonsterHPVisibility(ESlateVisibility::Collapsed);
	}


#if defined(WITH_EDITOR) || UE_BUILD_DEVELOPMENT
	if (TargetMonster)
	{
		//UE_LOG(LogARPG, Log, TEXT("Target Changed: %s"), *TargetMonster->GetName());
	}
	else
	{
		//UE_LOG(LogARPG, Log, TEXT("Target Cleared"));
	}
#endif
}

void UHUDWidget::OnRewardCardSelected(FRewardData RewardData)
{
	ADiaController* OwningController = Cast<ADiaController>(GetOwningPlayer());
	if (!IsValid(OwningController))
		return;

	bool SuccesApplyReward = OwningController->ApplyReward(RewardData);
}

void UHUDWidget::RegisteSkillOnQuickSlotWidget(int32 SkillID, int32 SlotIndex)
{
	if (SkillQuickSlotWidget)
	{
		SkillQuickSlotWidget->UpdateSkillSlot(SlotIndex, SkillID);
	}
}

void UHUDWidget::ShowClearAlret(FGuid RoomGuid)
{
	UDiaPrimaryLayout* Layout = DiaPrimaryLayout;
	if (!Layout)
	{
		UE_LOG(LogARPG, Warning, TEXT("UHUDWidget::ShowClearAlret - DiaPrimaryLayout is null"));
		return;
	}
	if (!ClearAlertWidgetClass)
	{
		UE_LOG(LogARPG, Warning, TEXT("UHUDWidget::ShowClearAlret - ClearAlertWidgetClass is null"));
		return;
	}
	UClearAlret* ClearAlretWidget = Cast<UClearAlret>(Layout->PushToLayer(FDiaGameplayTags::Get().UI_Layer_Menu, ClearAlertWidgetClass));
}

void UHUDWidget::ShowDamagePopup(float DamageAmount, const FVector2D& ScreenPos, bool bIsCritical)
{
	if (IsValid(DamagePopupWidgetClass))
	{
		UDiaDamagePopup* PopupWidget = CreateWidget<UDiaDamagePopup>(GetWorld(), DamagePopupWidgetClass);
		if (IsValid(PopupWidget))
		{
			PopupWidget->AddToViewport();
			PopupWidget->SetPositionInViewport(ScreenPos);
			PopupWidget->InitializeDamagePopup(DamageAmount);
		}
	}
	else
	{
		UE_LOG(LogARPG, Warning, TEXT("UHUDWidget::ShowDamagePopup - DamagePopupWidgetClass is null"));
	}
}

void UHUDWidget::OpenRewardChoicePanel(const FText& Title, const FText& Subtitle, const TArray<FRewardData>& RewardOptions)
{
	if (!IsValid(RewardChoicePanel))
	{
		UE_LOG(LogARPG, Warning, TEXT("UHUDWidget::OpenRewardChoicePanel - RewardChoicePanel is null"));
		return;
	}

	RewardChoicePanel->OpenRewardChoicePanel(Title, Subtitle, RewardOptions);
}

TOptional<FUIInputConfig> UHUDWidget::GetDesiredInputConfig() const
{
	return FUIInputConfig(
		ECommonInputMode::All,
		EMouseCaptureMode::CaptureDuringMouseDown,
		EMouseLockMode::LockOnCapture,
		false);
}

void UHUDWidget::BindCharacterAttributeChangeDelegates()
{
	if (CachedAbilitySystem->IsValidLowLevel())
	{
		//player는 이런식으로 설정한다 치는데, 몬스터는 어떻게?하지?
		HealthChangedDelegateHandle =
			CachedAbilitySystem
			->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetHealthAttribute())
			.AddUObject(this, &UHUDWidget::HandleHealthChanged);

		ManaChangedDelegateHandle =
			CachedAbilitySystem
			->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetManaAttribute())
			.AddUObject(this, &UHUDWidget::HandleManaChanged);

		ExpChangedDelegateHandle =
			CachedAbilitySystem->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetExpAttribute())
			.AddUObject(this, &UHUDWidget::HandlExpChanged);
	}
}

void UHUDWidget::UnBindCharacterAttributeChangeDelegates()
{
	if (CachedAbilitySystem->IsValidLowLevel())
	{
		CachedAbilitySystem
			->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetHealthAttribute())
			.Remove(HealthChangedDelegateHandle);
		HealthChangedDelegateHandle.Reset();
		CachedAbilitySystem
			->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetManaAttribute())
			.Remove(ManaChangedDelegateHandle);
		ManaChangedDelegateHandle.Reset();
		CachedAbilitySystem
			->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetExpAttribute())
			.Remove(ExpChangedDelegateHandle);
		ExpChangedDelegateHandle.Reset();
	}
}

void UHUDWidget::SetMonsterHPVisibility(ESlateVisibility _Visibility)
{
	MonsterHPWidget->SetVisibility(_Visibility);
	BossMonsterHPWidget->SetVisibility(_Visibility);
}


