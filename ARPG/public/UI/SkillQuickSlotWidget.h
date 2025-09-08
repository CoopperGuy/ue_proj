// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillQuickSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API USkillQuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	void NativeConstruct() override;

protected:

private:
	UPROPERTY(meta = (BindWidget))
	UUserWidget* SkillQuickSlot1;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* SkillQuickSlot2;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* SkillQuickSlot3;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* SkillQuickSlot4;
	UPROPERTY(meta = (BindWidget))
	UUserWidget* SkillQuickSlot5;

};
