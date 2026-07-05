// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Title/DiaLoadSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "System/DiaSaveSubsystem.h"

void UDiaLoadSlot::SetSlotIndex(int32 InSlotIndex)
{
	SlotIndex = InSlotIndex;

	if (SlotNumberText)
	{
		SlotNumberText->SetText(FText::AsNumber(SlotIndex + 1));
	}
}

void UDiaLoadSlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotButton)
	{
		SlotButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleClicked);
	}
}

void UDiaLoadSlot::HandleClicked()
{
	OnSaveSlotSelected.Broadcast(SlotIndex);
}

void UDiaLoadSlot::SaveGame(UDiaSaveSubsystem* SaveSubsystem)
{
	const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
	SaveSubsystem->SaveGame(SlotName);
}

void UDiaLoadSlot::LoadGame(UDiaSaveSubsystem* SaveSubsystem)
{
	const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
	SaveSubsystem->LoadGame(SlotName);
}

void UDiaLoadSlot::UpdateSlotUI(UDiaSaveSubsystem* SaveSubsystem)
{
	if (!IsValid(SaveSubsystem))
	{
		return;
	}

	const FString SlotName = FString::Printf(TEXT("SaveSlot_%d"), SlotIndex);
	const bool bSaveExists = SaveSubsystem->DoesSaveExist(SlotName);

	if (SlotNumberText)
	{
		SlotNumberText->SetText(FText::AsNumber(SlotIndex + 1));
	}

	if (SlotTitleText)
	{
		SlotTitleText->SetText(FText::FromString(FString::Printf(TEXT("Save Slot %d"), SlotIndex + 1)));
	}

	if (bSaveExists)
	{
		if (SlotMetaText)
		{
			SlotMetaText->SetText(FText::FromString(TEXT("Saved data")));
		}

		if (SlotTimeText)
		{
			SlotTimeText->SetText(FText::FromString(SlotName));
		}

		if (SlotStatusText)
		{
			SlotStatusText->SetText(FText::FromString(TEXT("READY")));
		}

		if (SlotButton)
		{
			SlotButton->SetIsEnabled(true);
		}
		return;
	}

	if (SlotMetaText)
	{
		SlotMetaText->SetText(FText::FromString(TEXT("Chapter --  |  Lv. --")));
	}

	if (SlotTimeText)
	{
		SlotTimeText->SetText(FText::FromString(TEXT("No save data")));
	}

	if (SlotStatusText)
	{
		SlotStatusText->SetText(FText::FromString(TEXT("EMPTY")));
	}

	if (SlotButton)
	{
		SlotButton->SetIsEnabled(bIsSaveMode);
	}
}

void UDiaLoadSlot::SetIsSaveMode(bool bSaveMode)
{
	bIsSaveMode = bSaveMode;
	// Update the UI based on the mode (save/load)
	// For example, you can change the button text or appearance here
}

void UDiaLoadSlot::OnSlotSelected()
{
	UDiaSaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UDiaSaveSubsystem>();
	if (!IsValid(SaveSubsystem))
		return;

	if (bIsSaveMode)
	{
		SaveGame(SaveSubsystem);
	}
	else
	{
		LoadGame(SaveSubsystem);
	}

	UpdateSlotUI(SaveSubsystem);
}
