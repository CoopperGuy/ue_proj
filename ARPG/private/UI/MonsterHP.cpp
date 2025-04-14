// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MonsterHP.h"
#include "Components/Image.h"

void UMonsterHP::NativeConstruct()
{
    Super::NativeConstruct();

    // �⺻ �Ķ���� �̸� ����
    if (percentageParamName.IsNone())
    {
        percentageParamName = FName("Percentage");
    }

    // HP ���� ��Ƽ���� �ʱ�ȭ
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
    // �� ���� ����
    float clampedValue = FMath::Clamp(percentage, 0.0f, 1.0f);

    // ��Ƽ���� �Ķ���� ������Ʈ
    if (IsValid(HPMaterial))
    {
        HPMaterial->SetScalarParameterValue(percentageParamName, clampedValue);
    }
}
