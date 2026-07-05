// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/DiaTitleController.h"
#include "UI/Title/DiaMainTitle.h"

void ADiaTitleController::BeginPlay()
{
	Super::BeginPlay();
	MainTitleWidget = LoadClass<UDiaMainTitle>(nullptr, TEXT("/Game/UI/Title/WBP_MainTitle.WBP_MainTitle_C"));
	if (MainTitleWidget)
	{
		UE_LOG(LogTemp, Log, TEXT("MainTitleWidget class loaded successfully."));
		MainTitleWidgetPtr = CreateWidget<UDiaMainTitle>(this, MainTitleWidget);
		if (MainTitleWidgetPtr)
		{
			UE_LOG(LogTemp, Log, TEXT("MainTitleWidget instance created successfully."));
			MainTitleWidgetPtr->AddToViewport();
		}
	}
}
