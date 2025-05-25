// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Types/ItemBase.h"

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

	void SpawnItemAtLocation(AActor* SpawnActor, const FItemBase& ItemData);
protected:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UHUDWidget> HUDWidgetClass;
    
    UPROPERTY()
    UHUDWidget* HUDWidgetInstance;
public:
    UHUDWidget* const GetHUDWidget() const { return HUDWidgetInstance; }
};