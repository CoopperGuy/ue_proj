// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Skill/DiaSkillType.h"
#include "Types/DiaGASSkillData.h"
#include "SkillPanelWidget.generated.h"

class UScrollBox;
class UCommonListView;
/**
 * 
 */
UCLASS()
class ARPG_API USkillPanelWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	void ToggleSkillPanel();

	void InitializeSkillPanel();
	void AddSkillToPanel(const FGASSkillData& SkillData, int32 SkillID, bool bIsActiveSkill);

	void RegisterSkillList(const TArray<class USkillObject*>& Skills);
protected:
	void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "SkillPanel")
	void HandleItemClicked(UObject* Item);
private:
	UPROPERTY(meta = (BindWidget))
	UCommonListView* ActiveSkillListView;

	UPROPERTY(meta = (BindWidget))
	UCommonListView* SubSkillListView;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Slot", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class USkillSlotWidget> SkillSlotWidgetClass;

};
