// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MonsterHP.h"
#include "Components/Image.h"

void UMonsterHP::NativeConstruct()
{
    Super::NativeConstruct();

    // 기본 파라미터 이름 설정
    if (percentageParamName.IsNone())
    {
        percentageParamName = FName("Percentage");
    }

    // HP 오브 머티리얼 초기화
    if (IsValid(HPGauge) /*&& IsValid(OrbImage->GetBrushResource())*/)
    {
        UMaterialInterface* baseMaterial = Cast<UMaterialInterface>(HPGauge->GetBrush().GetResourceObject());
        if (IsValid(baseMaterial))
        {
            HPMaterial = UMaterialInstanceDynamic::Create(baseMaterial, this);
            HPGauge->SetBrushFromMaterial(HPMaterial);
        }
    }


    UpdatePercentage(0.5f);
}

void UMonsterHP::UpdatePercentage(float percentage)
{
    // 값 범위 제한
    float clampedValue = FMath::Clamp(percentage, 0.0f, 1.0f);

    // 머티리얼 파라미터 업데이트
    if (IsValid(HPMaterial))
    {
        HPMaterial->SetScalarParameterValue(percentageParamName, clampedValue);
    }
}
