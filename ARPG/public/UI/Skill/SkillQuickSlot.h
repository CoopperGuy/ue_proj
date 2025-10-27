// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillQuickSlot.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class UProgressBar;
class UAbilitySystemComponent;
class UMaterialInstanceDynamic;
/**
 * 
 */
UCLASS()
class ARPG_API USkillQuickSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	void SetSkillQuickSlot(int32 InSkillID, UTexture2D* InSkillIcon, int32 InSkillLevel);
	void ClearSkillQuickSlot();
	void SetSkillID(int32 InSkillID) { SkillID = InSkillID; }
	void SetSkillIcon(UTexture2D* InSkillIcon);

	void UpdateCoolTime(UAbilitySystemComponent* ASC);

	void UpdateCoolDownPercentage(float Percentage);
	void UpdateSkillIcon(UTexture2D* InSkillIcon);
protected:
	private:
	UPROPERTY(meta = (BindWidget))
	UImage* SkillIconImage;
	UPROPERTY(meta = (BindWidget))
	UImage* SlotKey;
	UPROPERTY(meta = (BindWidget))
	UImage* SkillCoolTime;

	UPROPERTY()
    UMaterialInstanceDynamic* SkillIconMaterial;

	UPROPERTY()
    UMaterialInstanceDynamic* SkillCoolDownMateiral;

	UPROPERTY(EditDefaultsOnly, Category = "Material Parameters")
    FName skillParamIconName;

	UPROPERTY(EditDefaultsOnly, Category = "Material Parameters")
    FName coolDownParamName;

	int32 SkillID;
};
