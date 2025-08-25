// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "SkillPanelWidget.generated.h"

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
protected:
	void NativeConstruct() override;

private:
	
	
};
