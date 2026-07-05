// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "DiaLoadSlot.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveSlotSelected, int32, SlotIndex);

class UButton;
class UTextBlock;
class UDiaSaveSubsystem;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaLoadSlot : public UCommonUserWidget
{
	GENERATED_BODY()
	
	
public:
	void SetSlotIndex(int32 InSlotIndex);
	void SetIsSaveMode(bool bSaveMode);

	void OnSlotSelected();
	void UpdateSlotUI(UDiaSaveSubsystem* SaveSubsystem);
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleClicked();

	void SaveGame(UDiaSaveSubsystem* SaveSubsystem);
	void LoadGame(UDiaSaveSubsystem* SaveSubsystem);
public:
	UPROPERTY(BlueprintAssignable)
	FOnSaveSlotSelected OnSaveSlotSelected;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SlotButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotNumberText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotTitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotMetaText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotTimeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SlotStatusText;

private:
	int32 SlotIndex = INDEX_NONE;

	bool bIsSaveMode = false;
};
