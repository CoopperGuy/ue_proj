// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skill/SkillQuickSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

#include "AbilitySystemComponent.h"
#include "GAS/DiaGASHelper.h"

#include "Engine/Texture2D.h"


void USkillQuickSlot::NativeConstruct()
{
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
    }
}

void USkillQuickSlot::SetSkillQuickSlot(int32 InSkillID, UTexture2D* InSkillIcon, int32 InSkillLevel)
{
	SetSkillID(InSkillID);
	SetSkillIcon(InSkillIcon);
}

void USkillQuickSlot::ClearSkillQuickSlot()
{
	SetSkillID(-1);
	SetSkillIcon(nullptr);
}

void USkillQuickSlot::SetSkillIcon(UTexture2D* InSkillIcon)
{
	UpdateSkillIcon(InSkillIcon);
}

void USkillQuickSlot::UpdateCoolTime(UAbilitySystemComponent* ASC)
{
	if (!IsValid(ASC)) return;
	if (SkillID <= 0) return;

	float CoolTimeRatio = UDiaGASHelper::GetCooldownRatioBySkillID(ASC, SkillID);

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


