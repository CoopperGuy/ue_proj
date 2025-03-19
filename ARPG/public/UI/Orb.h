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
    // 초기화 함수
    virtual void NativeConstruct() override;
    
    // HP 값 업데이트 (0.0 ~ 1.0 사이 값)
    UFUNCTION(Category = "Status")
    void UpdatePercentage(float percentage);
protected:
    // 위젯 블루프린트에서 바인딩할 이미지 컴포넌트
    UPROPERTY(meta = (BindWidget))
    UImage* OrbImage;	

	UPROPERTY()
    UMaterialInstanceDynamic* OrbMaterial;

        // 퍼센티지 파라미터 이름
    UPROPERTY(EditDefaultsOnly, Category = "Material Parameters")
    FName percentageParamName;
};
