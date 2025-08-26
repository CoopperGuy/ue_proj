// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Skill/DiaSkillType.h"
#include "SkillPanelWidget.generated.h"

class UScrollBox;
/**
 * 
 */
UCLASS()
class ARPG_API USkillPanelWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	void ToggleSkillPanel();

	void RegisterSkillPanel(int32 SkillID);
	void UnregisterSkillPanel(int32 SkillID);

	void InitializeSkillPanel();
	void AddSkillToPanel(const FSkillData& SkillData, int32 SkillID, bool bIsActiveSkill);
protected:
	void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ActiveSkillScrollbar;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* SubSkillScrollBar;
};
