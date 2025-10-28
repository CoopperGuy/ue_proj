// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillQuickSlotWidget.generated.h"

class USkillQuickSlot;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class ARPG_API USkillQuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void UpdateSkillSlot(int32 SlotIndex, int32 SkillID);

	void StartCoolDownAnimation(int32 SlotIndex, UAbilitySystemComponent* ASC);
	void EndCoolDownAnimation(int32 SlotIndex);
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

	// 캐싱된 AbilitySystemComponent (성능 최적화)
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};
