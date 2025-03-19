// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DungeonGameMode.generated.h"

class UHUDWidget;

UCLASS()
class ARPG_API ADungeonGameMode : public AGameModeBase
{
    GENERATED_BODY()
    
public:
    ADungeonGameMode();
    
    virtual void BeginPlay() override;
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
    // HUD ���� Ŭ����
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UHUDWidget> HUDWidgetClass;
    
    // HUD ���� �ν��Ͻ�
    UPROPERTY()
    UHUDWidget* HUDWidgetInstance;
};