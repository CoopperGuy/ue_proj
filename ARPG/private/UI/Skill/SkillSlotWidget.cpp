// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skill/SkillSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void USkillSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
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
	SkillIconImage->SetBrushFromTexture(Icon);
}

void USkillSlotWidget::SetSkillName(const FText& Name)
{
	SkillNameText->SetText(Name);
}

void USkillSlotWidget::SetSkillLevel(int32 Level)
{
	SkillLevelText->SetText(FText::AsNumber(Level));
}
