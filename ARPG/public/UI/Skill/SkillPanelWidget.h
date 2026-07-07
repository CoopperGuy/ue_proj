// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Skill/DiaSkillType.h"
#include "Types/DiaGASSkillData.h"
#include "SkillPanelWidget.generated.h"

class UScrollBox;
class UCommonListView;
class USkillInfoObject;
class USkillObject;
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
	void AddSkillToPanel(const FGASSkillData& SkillData, int32 SkillID, int32 SkillLevel, bool bIsActiveSkill);

	void RegisterSkill(const USkillObject* Skill);
	void RegisterSkillList(const TArray<USkillObject*>& Skills);
	bool UpdateSkillLevel(int32 SkillID, int32 NewLevel);
	void RegisterSkillVariant(int32 SkillID, int32 VariantID);
protected:
	void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "SkillPanel")
	void HandleItemClicked(UObject* Item);
private:
	USkillInfoObject* FindActiveSkillInfoObject(int32 SkillID) const;
	void RefreshOwnedSkillVariants(int32 SkillID);

	UPROPERTY(meta = (BindWidget))
	UCommonListView* ActiveSkillListView;

	UPROPERTY(meta = (BindWidget))
	UCommonListView* SubSkillListView;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Slot", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class USkillSlotWidget> SkillSlotWidgetClass;

	TMap<int32, TWeakObjectPtr<USkillInfoObject>> RegisteredSkillInfoMap;
	int32 CurrentSelectedSkillID = INDEX_NONE;

};
