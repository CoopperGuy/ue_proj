// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Orb.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UOrb::NativeConstruct()
{
    Super::NativeConstruct();

    // 기본 파라미터 이름 설정
    if (percentageParamName.IsNone())
    {
        percentageParamName = FName("Percentage");
    }

    // HP 오브 머티리얼 초기화
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
    // 값 범위 제한
    float clampedValue = FMath::Clamp(percentage, 0.0f, 1.0f);

    // 머티리얼 파라미터 업데이트
    if (IsValid(OrbMaterial))
    {
        OrbMaterial->SetScalarParameterValue(percentageParamName, clampedValue);
    }
}
