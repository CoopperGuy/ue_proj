// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Menu/DiaMenuSystem.h"
#include "Components/Button.h"
#include "UI/Title/SlotSelectPannel.h"


void UDiaMenuSystem::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddDynamic(this, &ThisClass::OnResumeButtonClicked);
	}

	if(SaveGameButton)
	{
		SaveGameButton->OnClicked.AddDynamic(this, &ThisClass::OnSaveGameButtonClicked);
	}

	if(LoadGameButton)
	{
		LoadGameButton->OnClicked.AddDynamic(this, &ThisClass::OnLoadGameButtonClicked);
	}

	if(SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &ThisClass::OnSettingsButtonClicked);
	}

	if(MainMenuButton)
	{
		MainMenuButton->OnClicked.AddDynamic(this, &ThisClass::OnMainMenuButtonClicked);
	}

	if(QuitGameButton)
	{
		QuitGameButton->OnClicked.AddDynamic(this, &ThisClass::OnQuitGameButtonClicked);
	}

	if (SlotSelectPannel)
	{
		SlotSelectPannel->OnCloseSlotSelectPanel.AddDynamic(this, &ThisClass::OnSlotSelectBackClicked);
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
