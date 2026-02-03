// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "SkillSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UTexture2D;


UCLASS()
class ARPG_API USkillInfoObject : public UObject
{
	GENERATED_BODY()
public:
	int32 SkillID;
	UPROPERTY()
	UTexture2D* SkillIcon;
	FText SkillName;
	int32 SkillLevel;
};


/**
 * 
 */
UCLASS()
class ARPG_API USkillSlotWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	void SetSkillInfo(int32 ID, UTexture2D* Icon, const FText& Name, int32 Level);

	void SetSkillID(int32 ID) { SkillID = ID; }

	void SetSkillIcon(UTexture2D* Icon);
	void SetSkillName(const FText& Name);
	void SetSkillLevel(int32 Level);
protected:
	UFUNCTION()
	void OnApplyButtonClicked();
	UFUNCTION()
	void OnCancelButton();
private:
	UPROPERTY(meta = (BindWidget))
	UImage* SkillIconImage;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillNameText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SkillLevelText;

	UPROPERTY(meta = (BindWidget))
	UButton* ApplyBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelBtn;

	int32 SkillID;

	UPROPERTY()
	TWeakObjectPtr<USkillInfoObject> SkillInfoObjectPtr;
};
