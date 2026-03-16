// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Containers/UnrealString.h"
#include "ClearAlret.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UClearAlret : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

	void HandleDeactivate();
protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ClearText;

	UPROPERTY()
	FTimerHandle DeactivateTimerHandle;
};
