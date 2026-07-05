// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "DiaMainTitle.generated.h"

class UButton;
class UWidget;
class USlotSelectPannel;
/**
 *
 */
UCLASS()
class ARPG_API UDiaMainTitle : public UCommonUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnNewGameClicked();

	UFUNCTION()
	void OnLoadGameClicked();

	UFUNCTION()
	void OnSlotSelectBackClicked();
protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NewGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> LoadGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> MenuPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlotSelectPannel> SlotSelectPannel;

	UPROPERTY()
	TArray<TObjectPtr<UButton>> SlotButtons;

};
