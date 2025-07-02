// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Orb.generated.h"

class UImage;
class UMaterialInstanceDynamic;
/**
 * 
 */
UCLASS()
class ARPG_API UOrb : public UUserWidget
{
	GENERATED_BODY()
public:
    // �ʱ�ȭ �Լ�
    virtual void NativeConstruct() override;
    
    // HP �� ������Ʈ (0.0 ~ 1.0 ���� ��)
    UFUNCTION(Category = "Status")
    void UpdatePercentage(float percentage);
protected:
    // ���� �������Ʈ���� ���ε��� �̹��� ������Ʈ
    UPROPERTY(meta = (BindWidget))
    UImage* OrbImage;	

	UPROPERTY()
    UMaterialInstanceDynamic* OrbMaterial;

        // �ۼ�Ƽ�� �Ķ���� �̸�
    UPROPERTY(EditDefaultsOnly, Category = "Material Parameters")
    FName percentageParamName;
};
