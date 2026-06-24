// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "DiaItemDebugWidget.generated.h"

class UDiaEquipmentComponent;
class UDiaOptionManagerComponent;
class UAbilitySystemComponent;

class UTextBlock;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaItemDebugWidget : public UCommonUserWidget
{
	GENERATED_BODY()	
public:
    void SetDebugSource(
        UDiaEquipmentComponent* InEquipmentComponent,
        UDiaOptionManagerComponent* InOptionManagerComponent,
        UAbilitySystemComponent* InASC
    );
    void RefreshDebugText();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> DebugText;

	UPROPERTY()
	TObjectPtr<UDiaEquipmentComponent> EquipmentComponent;

	UPROPERTY()
	TObjectPtr<UDiaOptionManagerComponent> OptionManagerComponent;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	float RefreshAccumulator = 0.0f;
};
