// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Containers/UnrealString.h"
#include "ClearAlret.generated.h"

class UWidgetAnimation;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Clear Alert", meta = (ClampMin = "0.0"))
	float DisplayDuration = 1.2f;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* Anim_FadeOut;
};
