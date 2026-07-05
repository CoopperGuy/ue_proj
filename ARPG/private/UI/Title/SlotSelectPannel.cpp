// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Title/SlotSelectPannel.h"
#include "UI/Title/DiaLoadSlot.h"
#include "System/DiaSaveSubsystem.h"

void USlotSelectPannel::NativeConstruct()
{
	Super::NativeConstruct();

	SlotButtons = { Slot1Button, Slot2Button, Slot3Button };

	for (int32 i = 0; i < SlotButtons.Num(); ++i)
	{
		if (!IsValid(SlotButtons[i]))
		{
			continue;
		}

		SlotButtons[i]->SetSlotIndex(i);
		SlotButtons[i]->OnSaveSlotSelected.AddUniqueDynamic(this, &ThisClass::HandleClicked);
	}

	BackButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleSlotSelectBackButtonClicked);
}

void USlotSelectPannel::HandleClicked(int32 SlotIndex)
{
	// Handle the clicked slot index here
	UE_LOG(LogTemp, Log, TEXT("Slot %d clicked"), SlotIndex);

	if (!SlotButtons.IsValidIndex(SlotIndex) || !IsValid(SlotButtons[SlotIndex]))
	{
		return;
	}

	SlotButtons[SlotIndex]->OnSlotSelected();
	UpdateSlotButtons();
}

void USlotSelectPannel::HandleSlotSelectBackButtonClicked()
{
	OnCloseSlotSelectPanel.Broadcast();
}

void USlotSelectPannel::OpenSlotSelectPanel(bool bSaveMode)
{
	UDiaSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UDiaSaveSubsystem>();

	bIsSaveMode = bSaveMode;
	// Update the UI based on the mode (save/load)
	for (UDiaLoadSlot* SlotButton : SlotButtons)
	{
		if (IsValid(SlotButton))
		{
			SlotButton->SetIsSaveMode(bIsSaveMode);
		}
	}
	UpdateSlotButtons();
	SetVisibility(ESlateVisibility::Visible);
}

void USlotSelectPannel::UpdateSlotButtons()
{
	UDiaSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UDiaSaveSubsystem>();
	if (!IsValid(SaveSubsystem))
	{
		return;
	}

	for (UDiaLoadSlot* SlotButton : SlotButtons)
	{
		if (IsValid(SlotButton))
		{
			SlotButton->UpdateSlotUI(SaveSubsystem);
		}
	}
}
