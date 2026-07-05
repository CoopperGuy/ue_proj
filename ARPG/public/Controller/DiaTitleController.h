// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DiaTitleController.generated.h"

class UDiaMainTitle;
/**
 * 
 */
UCLASS()
class ARPG_API ADiaTitleController : public APlayerController
{
	GENERATED_BODY()
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	TSubclassOf<UDiaMainTitle> MainTitleWidget;
	
	UPROPERTY()
	TObjectPtr<UDiaMainTitle> MainTitleWidgetPtr;
};
