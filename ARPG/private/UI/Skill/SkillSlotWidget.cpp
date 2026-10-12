// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skill/SkillSlotWidget.h"
#include "UI/Skill/SkillToolTipWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Character/DiaCharacter.h"
#include "Logging/ARPGLogChannels.h"

void USkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible);
	RefreshToolTip();

	if (IsValid(ApplyBtn))
	{
		ApplyBtn->OnClicked.AddUniqueDynamic(this, &USkillSlotWidget::OnApplyButtonClicked);
	}
	if (IsValid(CancelBtn))
	{
		CancelBtn->OnClicked.AddUniqueDynamic(this, &USkillSlotWidget::OnCancelButton);
	}
}

void USkillSlotWidget::SetSkillInfo(int32 ID, UTexture2D* Icon, const FText& Name, int32 Level)
{
	SetSkillID(ID);
	SetSkillIcon(Icon);
	SetSkillName(Name);
	SetSkillLevel(Level);
}

void USkillSlotWidget::SetSkillIcon(UTexture2D* Icon)
{
	if (!IsValid(SkillIconImage))
	{
		return;
	}
	SkillIconImage->SetBrushFromTexture(Icon);
}

void USkillSlotWidget::SetSkillName(const FText& Name)
{
	if (!IsValid(SkillNameText))
	{
		return;
	}
	SkillNameText->SetText(Name);
}

void USkillSlotWidget::SetSkillLevel(int32 Level)
{
    DisplayedSkillLevel = FMath::Max(1, Level);
    if (SkillLevelText) SkillLevelText->SetText(FText::AsNumber(DisplayedSkillLevel));
    RefreshToolTip();
}

void USkillSlotWidget::OnApplyButtonClicked()
{
	APawn* Pawn = GetOwningPlayerPawn();
	ADiaCharacter* Character = Cast<ADiaCharacter>(Pawn);
	if (!Character || !SkillInfoObjectPtr.IsValid())
	{
		return;
	}

	UE_LOG(LogARPG, Log, TEXT("USkillSlotWidget::OnApplyButtonClicked - Applying SkillID: %d"), SkillID);

	if (SkillInfoObjectPtr->isMainSkill)
	{
		Character->TryAutoAssignSkillToQuickSlot(SkillID);
	}
	else
	{
		Character->AddSkillVariantToSkillObjcet(MainSkillID, SkillID, true);
	}
	RefreshToolTip();
}

void USkillSlotWidget::OnCancelButton()
{
	APawn* Pawn = GetOwningPlayerPawn();
	ADiaCharacter* Character = Cast<ADiaCharacter>(Pawn);
	if (!Character || !SkillInfoObjectPtr.IsValid())
	{
		return;
	}

	UE_LOG(LogARPG, Log, TEXT("USkillSlotWidget::OnCancelButton - Canceling SkillID: %d"), SkillID);

	if (SkillInfoObjectPtr->isMainSkill)
	{
		Character->ClearSkillFromQuickSlot(SkillID);
	}
	else
	{
		Character->AddSkillVariantToSkillObjcet(MainSkillID, SkillID, false);
	}
	RefreshToolTip();
}

void USkillSlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    UpdateSkillInfoObject(Cast<USkillInfoObject>(ListItemObject));
    IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
}

void USkillSlotWidget::NativeOnEntryReleased()
{
    ClearSkillInfo();
    IUserObjectListEntry::NativeOnEntryReleased();
}

void USkillSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    RefreshToolTip();
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
}

void USkillSlotWidget::ClearSkillInfo()
{
    SkillInfoObjectPtr.Reset();
    SkillID = MainSkillID = INDEX_NONE;
    DisplayedSkillLevel = 1;
    SetToolTip(nullptr);
    SkillToolTip = nullptr;
    SetSkillIcon(nullptr);
    SetSkillName(FText::GetEmpty());
    if (SkillLevelText) SkillLevelText->SetText(FText::GetEmpty());
}

void USkillSlotWidget::RefreshToolTip()
{
    if (SkillID <= 0)
    {
        SetToolTip(nullptr);
        return;
    }
    if (!SkillToolTip) SkillToolTip = CreateWidget<USkillToolTipWidget>(this, USkillToolTipWidget::StaticClass());
    const bool bVariant = SkillInfoObjectPtr.IsValid() && !SkillInfoObjectPtr->isMainSkill;
    const ESkillToolTipContext Context = bVariant ? ESkillToolTipContext::VariantList : ESkillToolTipContext::SkillList;
    if (SkillToolTip && SkillToolTip->SetSkill(SkillID, Context, MainSkillID, DisplayedSkillLevel))
        SetToolTip(SkillToolTip);
    else
        SetToolTip(nullptr);
}

void USkillSlotWidget::UpdateSkillInfoObject(USkillInfoObject* InSkillInfoObjectPtr)
{
	if (!IsValid(InSkillInfoObjectPtr))
	{
        ClearSkillInfo();
		return;
	}

	SkillInfoObjectPtr = InSkillInfoObjectPtr;
	MainSkillID = InSkillInfoObjectPtr->MainSkillID;
	SetSkillInfo(InSkillInfoObjectPtr->SkillID, InSkillInfoObjectPtr->SkillIcon, InSkillInfoObjectPtr->SkillName, InSkillInfoObjectPtr->SkillLevel);
}
