// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillQuickSlotWidget.generated.h"

class USkillQuickSlot;
/**
 * 
 */
UCLASS()
class ARPG_API USkillQuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	void NativeConstruct() override;

	void UpdateSkillSlot(int32 SlotIndex, int32 SkillID);
protected:
	
private:
	UPROPERTY(meta = (BindWidget))
	USkillQuickSlot* SkillQuickSlot1;
	UPROPERTY(meta = (BindWidget))
	USkillQuickSlot* SkillQuickSlot2;
	UPROPERTY(meta = (BindWidget))
	USkillQuickSlot* SkillQuickSlot3;
	UPROPERTY(meta = (BindWidget))
	USkillQuickSlot* SkillQuickSlot4;
	UPROPERTY(meta = (BindWidget))
	USkillQuickSlot* SkillQuickSlot5;

	TArray<USkillQuickSlot*> SkillSlots;
};
