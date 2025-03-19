// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DungeonGameMode.h"
#include "Character/DiaCharacter.h"
#include "UI/HUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ADungeonGameMode::ADungeonGameMode()
{
    // 기본 클래스 설정
    DefaultPawnClass = ADiaCharacter::StaticClass();
    
    // 기본 HUD 위젯 클래스 설정 (블루프린트에서 오버라이드 가능)
    static ConstructorHelpers::FClassFinder<UHUDWidget> HUDWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_HUDWidget.WBP_HUDWidget"));
    if (HUDWidgetClassFinder.Succeeded())
    {
        HUDWidgetClass = HUDWidgetClassFinder.Class;
    }
}

void ADungeonGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // HUD 위젯 생성 및 표시
    if (HUDWidgetClass)
    {
        APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (IsValid(playerController))
        {
            HUDWidgetInstance = CreateWidget<UHUDWidget>(playerController, HUDWidgetClass);
            if (IsValid(HUDWidgetInstance))
            {
                HUDWidgetInstance->AddToViewport();
            }
        }
    }
}

void ADungeonGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
}
