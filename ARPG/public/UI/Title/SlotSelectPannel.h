// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "SlotSelectPannel.generated.h"

class UDiaLoadSlot;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCloseSlotSelectPanelDelegate);

/**
 * 
 */
UCLASS()
class ARPG_API USlotSelectPannel : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION()

	void HandleClicked(int32 SlotIndex);

	UFUNCTION()
	void HandleSlotSelectBackButtonClicked();

	void OpenSlotSelectPanel(bool bSaveMode);
	void CloseSlotSelectPanel() { SetVisibility(ESlateVisibility::Hidden); }
protected:
	void UpdateSlotButtons();
public:
	UPROPERTY(BlueprintAssignable, Category = "SlotSelectPannel")
	FOnCloseSlotSelectPanelDelegate OnCloseSlotSelectPanel;
private:
	UPROPERTY()
	TArray<UDiaLoadSlot*> SlotButtons;

	UPROPERTY(meta = (BindWidget))
	UDiaLoadSlot* Slot1Button;

	UPROPERTY(meta = (BindWidget))
	UDiaLoadSlot* Slot2Button;

	UPROPERTY(meta = (BindWidget))
	UDiaLoadSlot* Slot3Button;

	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;
	
	bool bIsSaveMode = false;

public:
	void SetIsSaveMode(bool bSaveMode) { bIsSaveMode = bSaveMode; }
};
