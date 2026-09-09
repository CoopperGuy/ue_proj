// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Title/DiaMainTitle.h"
#include "Components/Button.h"
#include "Components/Widget.h"

#include "UI/Title/SlotSelectPannel.h"

#include "Kismet/GameplayStatics.h"


void UDiaMainTitle::NativeConstruct()
{
	Super::NativeConstruct();


	if (MenuPanel)
	{
		MenuPanel->SetVisibility(ESlateVisibility::Visible);
	}

	if (SlotSelectPannel)
	{
		SlotSelectPannel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (NewGameButton)
	{
		NewGameButton->OnClicked.AddUniqueDynamic(this, &UDiaMainTitle::OnNewGameClicked);
	}

	if (LoadGameButton)
	{
		LoadGameButton->OnClicked.AddUniqueDynamic(this, &UDiaMainTitle::OnLoadGameClicked);
	}

	if (SlotSelectPannel)
	{
		SlotSelectPannel->OnCloseSlotSelectPanel.AddUniqueDynamic(this, &UDiaMainTitle::OnSlotSelectBackClicked);
	}
}

void UDiaMainTitle::OnNewGameClicked()
{
	UGameplayStatics::OpenLevel(this, FName("TestRoom"));
}

void UDiaMainTitle::OnLoadGameClicked()
{
	if (MenuPanel)
	{
		MenuPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SlotSelectPannel)
	{
		SlotSelectPannel->OpenSlotSelectPanel(false); // Set to load mode
	}
}

void UDiaMainTitle::OnSlotSelectBackClicked()
{
	if(SlotSelectPannel)
	{
		SlotSelectPannel->CloseSlotSelectPanel();
	}
	if (MenuPanel)
	{
		MenuPanel->SetVisibility(ESlateVisibility::Visible);
	}
}
