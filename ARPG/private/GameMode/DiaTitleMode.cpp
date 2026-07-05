// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DiaTitleMode.h"
#include "Controller/DiaTitleController.h"

ADiaTitleMode::ADiaTitleMode()
{
	PlayerControllerClass = ADiaTitleController::StaticClass();
	//// 기본 클래스 설정
	//DefaultPawnClass = ADiaCharacter::StaticClass();
	//GameStateClass = ADiaGameState::StaticClass();

	//// 기본 HUD 위젯 클래스 설정 (블루프린트에서 오버라이드 가능)
	//// Blueprint 생성 클래스는 이름 뒤에 _C 접미사가 붙음
	//static ConstructorHelpers::FClassFinder<UHUDWidget> HUDWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_HUDWidget.WBP_HUDWidget_C"));
	//if (HUDWidgetClassFinder.Succeeded())
	//{
	//	HUDWidgetClass = HUDWidgetClassFinder.Class;
	//}
}

void ADiaTitleMode::BeginPlay()
{
	Super::BeginPlay();
}

void ADiaTitleMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}
