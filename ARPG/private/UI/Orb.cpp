// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Orb.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UOrb::NativeConstruct()
{
    Super::NativeConstruct();

    if (percentageParamName.IsNone())
    {
        percentageParamName = FName("Percentage");
    }

    if (IsValid(OrbImage) /*&& IsValid(OrbImage->GetBrushResource())*/)
    {
        UMaterialInterface* baseMaterial = Cast<UMaterialInterface>(OrbImage->GetBrush().GetResourceObject());
        if (IsValid(baseMaterial))
        {
            OrbMaterial = UMaterialInstanceDynamic::Create(baseMaterial, this);
            OrbImage->SetBrushFromMaterial(OrbMaterial);
        }
    }


    UpdatePercentage(1.f);
}

void UOrb::UpdatePercentage(float percentage)
{
    float clampedValue = FMath::Clamp(percentage, 0.0f, 1.0f);

    if (IsValid(OrbMaterial))
    {
        OrbMaterial->SetScalarParameterValue(percentageParamName, clampedValue);
    }
}
