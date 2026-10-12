// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skill/SkillQuickSlot.h"
#include "UI/Skill/SkillToolTipWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

#include "AbilitySystemComponent.h"
#include "GAS/DiaGASHelper.h"

#include "Engine/Texture2D.h"


void USkillQuickSlot::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::Visible);
    RefreshToolTip();
    if (skillParamIconName.IsNone())
    {
        skillParamIconName = FName("SkillIcon");
    }

    if (coolDownParamName.IsNone())
    {
        coolDownParamName = FName("SkillCoolDown");
    }

    if (IsValid(SkillIconImage))
    {
        UMaterialInterface* baseMaterial = Cast<UMaterialInterface>(SkillIconImage->GetBrush().GetResourceObject());
        if (IsValid(baseMaterial))
        {
            SkillIconMaterial = UMaterialInstanceDynamic::Create(baseMaterial, this);
            SkillIconImage->SetBrushFromMaterial(SkillIconMaterial);
        }
    }

    if (IsValid(SkillCoolTime))
    {
        UMaterialInterface* baseMaterial = Cast<UMaterialInterface>(SkillCoolTime->GetBrush().GetResourceObject());
        if (IsValid(baseMaterial))
        {
            SkillCoolDownMateiral = UMaterialInstanceDynamic::Create(baseMaterial, this);
            SkillCoolTime->SetBrushFromMaterial(SkillCoolDownMateiral);
        }
        SkillCoolTime->SetVisibility(ESlateVisibility::Collapsed);
    }

    UpdateCoolDownPercentage(0.f);
}

void USkillQuickSlot::SetSkillQuickSlot(int32 InSkillID, UTexture2D* InSkillIcon, int32 InSkillLevel)
{
	SkillLevel = FMath::Max(1, InSkillLevel);
	SetSkillID(InSkillID);
	SetSkillIcon(InSkillIcon);
}

void USkillQuickSlot::ClearSkillQuickSlot()
{
	SkillLevel = 1;
	SetSkillID(-1);
	SetSkillIcon(nullptr);
    UpdateCoolDownPercentage(0.f);
    if (SkillCoolTime) SkillCoolTime->SetVisibility(ESlateVisibility::Collapsed);
}

void USkillQuickSlot::SetSkillIcon(UTexture2D* InSkillIcon)
{
	UpdateSkillIcon(InSkillIcon);
}

void USkillQuickSlot::UpdateCoolTime(UAbilitySystemComponent* ASC)
{
    if (!SkillCoolTime) return;
    if (!IsValid(ASC))
    {
        UpdateCoolDownPercentage(0.f);
        SkillCoolTime->SetVisibility(ESlateVisibility::Collapsed);
		return;
    }
    
	if (SkillID <= 0) return;

	float CoolTimeRatio = UDiaGASHelper::GetCooldownRatioBySkillID(ASC, SkillID);

	// 쿨타임이 있을 때만 표시
	if (CoolTimeRatio > 0.0f)
	{
		SkillCoolTime->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SkillCoolTime->SetVisibility(ESlateVisibility::Collapsed);
	}

	UpdateCoolDownPercentage(CoolTimeRatio);
}

void USkillQuickSlot::UpdateCoolDownPercentage(float Percentage)
{
    float clampedValue = FMath::Clamp(Percentage, 0.0f, 1.0f);
    if (IsValid(SkillCoolDownMateiral))
    {
        SkillCoolDownMateiral->SetScalarParameterValue(coolDownParamName, clampedValue);
	}
}

void USkillQuickSlot::UpdateSkillIcon(UTexture2D* InSkillIcon)
{
    if (IsValid(SkillIconMaterial))
    {
        SkillIconMaterial->SetTextureParameterValue(skillParamIconName, InSkillIcon);
	}
}


void USkillQuickSlot::SetSkillID(int32 InSkillID)
{
    SkillID = InSkillID;
    RefreshToolTip();
}

void USkillQuickSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    RefreshToolTip();
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
}

void USkillQuickSlot::NativeDestruct()
{
    SetToolTip(nullptr);
    SkillToolTip = nullptr;
    Super::NativeDestruct();
}

void USkillQuickSlot::RefreshToolTip()
{
    if (SkillID <= 0)
    {
        SetToolTip(nullptr);
        SkillToolTip = nullptr;
        return;
    }
    if (!SkillToolTip) SkillToolTip = CreateWidget<USkillToolTipWidget>(this, USkillToolTipWidget::StaticClass());
    if (SkillToolTip && SkillToolTip->SetSkill(SkillID, ESkillToolTipContext::QuickSlot, INDEX_NONE, SkillLevel))
        SetToolTip(SkillToolTip);
    else
        SetToolTip(nullptr);
}
