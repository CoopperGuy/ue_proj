// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/DiaMenuSystem.h"
#include "Components/Button.h"
#include "UI/Title/SlotSelectPannel.h"


void UDiaMenuSystem::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnResumeButtonClicked);
	}

	if(SaveGameButton)
	{
		SaveGameButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnSaveGameButtonClicked);
	}

	if(LoadGameButton)
	{
		LoadGameButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnLoadGameButtonClicked);
	}

	if(SettingsButton)
	{
		SettingsButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnSettingsButtonClicked);
	}

	if(MainMenuButton)
	{
		MainMenuButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnMainMenuButtonClicked);
	}

	if(QuitGameButton)
	{
		QuitGameButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnQuitGameButtonClicked);
	}

	if (SlotSelectPannel)
	{
		SlotSelectPannel->OnCloseSlotSelectPanel.AddUniqueDynamic(this, &ThisClass::OnSlotSelectBackClicked);
	}
}

void UDiaMenuSystem::OnResumeButtonClicked()
{
}

void UDiaMenuSystem::OnSaveGameButtonClicked()
{
	if (IsValid(SaveGameButton) && IsValid(SlotSelectPannel))
	{
		SlotSelectPannel->OpenSlotSelectPanel(true);
	}
}

void UDiaMenuSystem::OnLoadGameButtonClicked()
{
	if (IsValid(LoadGameButton) && IsValid(SlotSelectPannel))
	{
		SlotSelectPannel->OpenSlotSelectPanel(false);
	}
}

void UDiaMenuSystem::OnSettingsButtonClicked()
{
}

void UDiaMenuSystem::OnMainMenuButtonClicked()
{
}

void UDiaMenuSystem::OnQuitGameButtonClicked()
{
}

void UDiaMenuSystem::OnSlotSelectBackClicked()
{
	if (SlotSelectPannel)
	{
		SlotSelectPannel->CloseSlotSelectPanel();
	}
}
