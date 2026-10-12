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
class USkillToolTipWidget;


UCLASS()
class ARPG_API USkillInfoObject : public UObject
{
	GENERATED_BODY()
public:
	int32 SkillID = -1;
	UPROPERTY()
	UTexture2D* SkillIcon = nullptr;
	FText SkillName;
	int32 SkillLevel = 1;

	bool isMainSkill = true;
	int32 MainSkillID = -1;
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
	virtual void NativeOnEntryReleased() override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	void UpdateSkillInfoObject(USkillInfoObject* InSkillInfoObjectPtr);
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
	void RefreshToolTip();
	void ClearSkillInfo();

	UPROPERTY(Transient)
	TObjectPtr<USkillToolTipWidget> SkillToolTip;
	int32 DisplayedSkillLevel = 1;
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

	int32 SkillID = -1;
	int32 MainSkillID = -1;

	UPROPERTY()
	TWeakObjectPtr<USkillInfoObject> SkillInfoObjectPtr;
};
