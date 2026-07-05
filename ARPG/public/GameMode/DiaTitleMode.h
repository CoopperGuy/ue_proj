// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DiaTitleMode.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API ADiaTitleMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ADiaTitleMode();
	virtual void BeginPlay() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;


};
