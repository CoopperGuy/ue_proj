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
class UDiaSkillManagerComponent;
class UAbilitySystemComponent;
class ADiaBaseCharacter;
class ADiaController;
/**
 * 
 */
UCLASS()
class ARPG_API USkillPanelWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "SkillPanel")
	void HandleItemClicked(UObject* Item);
private:
	UDiaSkillManagerComponent* GetSkillManagerComponent() const;
	bool TryInitializeFromPlayer();
	void BindPlayerASCReadyEvent();
	void UnbindPlayerASCReadyEvent();
	void HandlePlayerASCReady(ADiaBaseCharacter* PlayerCharacter, UAbilitySystemComponent* AbilitySystem);
	void BindSkillManagerDelegates();
	void UnbindSkillManagerDelegates();
	void InitializeSkillPanel();
	void AddSkillToPanel(const FGASSkillData& SkillData, int32 SkillID, int32 SkillLevel, bool bIsActiveSkill);
	void RegisterSkill(const USkillObject* Skill);
	void RegisterSkillList(const TArray<USkillObject*>& Skills);
	bool UpdateSkillLevel(int32 SkillID, int32 NewLevel);
	void RegisterSkillVariant(int32 SkillID, int32 VariantID);
	void HandleSkillRegistered(int32 SkillID, int32 SlotIndex);
	void HandleSkillLevelChanged(int32 SkillID, int32 NewLevel);
	void HandleSkillVariantAdded(int32 SkillID, int32 VariantID);
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

	UPROPERTY(Transient)
	TWeakObjectPtr<ADiaController> BoundPlayerController;

	FDelegateHandle PlayerASCReadyDelegateHandle;

	UPROPERTY(Transient)
	TWeakObjectPtr<UDiaSkillManagerComponent> BoundSkillManager;

	FDelegateHandle SkillRegisteredHandle;
	FDelegateHandle SkillLevelChangedHandle;
	FDelegateHandle SkillVariantAddedHandle;
};
