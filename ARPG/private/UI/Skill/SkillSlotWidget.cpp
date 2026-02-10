// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skill/SkillSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Character/DiaCharacter.h"

void USkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ApplyBtn->OnClicked.AddDynamic(this, &USkillSlotWidget::OnApplyButtonClicked);
	CancelBtn->OnClicked.AddDynamic(this, &USkillSlotWidget::OnCancelButton);
}

void USkillSlotWidget::SetSkillInfo(int32 ID, UTexture2D* Icon, const FText& Name, int32 Level)
{
	if (!IsValid(SkillIconImage) || !IsValid(SkillNameText) || !IsValid(SkillLevelText))
	{
		return;
	}

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
	if (!IsValid(SkillLevelText))
	{
		return;
	}
	SkillLevelText->SetText(FText::AsNumber(Level));
}

void USkillSlotWidget::OnApplyButtonClicked()
{
	APawn* Pawn = GetOwningPlayerPawn();
	ADiaCharacter* Character = Cast<ADiaCharacter>(Pawn);
	if (!Character)
		return;

	UE_LOG(LogTemp, Log, TEXT("USkillSlotWidget::OnApplyButtonClicked - Applying SkillID: %d"), SkillID);

	if (SkillInfoObjectPtr->isMainSkill)
	{
		Character->SetSkillIDOnQuickSlotWidget(SkillID, 0); // 0은 예시로 첫 번째 슬롯에 할당
	}
	else
	{
		Character->AddSkillVariantToSkillObjcet(MainSkillID, SkillID, true); // VariantID를 SkillID로 가정
	}

	return;
}

void USkillSlotWidget::OnCancelButton()
{
	APawn* Pawn = GetOwningPlayerPawn();
	ADiaCharacter* Character = Cast<ADiaCharacter>(Pawn);
	if (!Character)
		return;

	UE_LOG(LogTemp, Log, TEXT("USkillSlotWidget::OnApplyButtonClicked - Applying SkillID: %d"), SkillID);

	if (SkillInfoObjectPtr->isMainSkill)
	{
		Character->SetSkillIDOnQuickSlotWidget(-1, 0); // 0은 예시로 첫 번째 슬롯에 할당

	}
	else
	{
		Character->AddSkillVariantToSkillObjcet(MainSkillID, SkillID, false); // VariantID를 SkillID로 가정
	}

	return;
}

void USkillSlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	if (!IsValid(ListItemObject))
	{
		return;
	}

	if (USkillInfoObject* SkillInfoObject = Cast<USkillInfoObject>(ListItemObject))
	{
		SkillInfoObjectPtr = SkillInfoObject;
		SkillInfoObjectPtr->isMainSkill = SkillInfoObject->isMainSkill;
		SkillInfoObjectPtr->MainSkillID = SkillInfoObject->MainSkillID;
		MainSkillID = SkillInfoObject->MainSkillID;
		SetSkillInfo(SkillInfoObject->SkillID, SkillInfoObject->SkillIcon, SkillInfoObject->SkillName, SkillInfoObject->SkillLevel);
	}
}
