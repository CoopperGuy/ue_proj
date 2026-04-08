// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "DiaNameWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaNameWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UDiaNameWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void InteractButtonClicked();
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameText;	
};
