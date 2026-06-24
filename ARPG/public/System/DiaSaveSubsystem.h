// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiaSaveSubsystem.generated.h"

class UDiaSaveGame;
class ADiaController;
class ADiaBaseCharacter;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	bool SaveGame(const FString& SlotName);
	bool LoadGame(const FString& SlotName);
	bool DoesSaveExist(const FString& SlotName) const;
	void DeleteSave(const FString& SlotName);	

	void FillSaveDataByGame(UDiaSaveGame* SaveGameInstance, const ADiaController* Controller);
	void ApplySaveDataToGame(const UDiaSaveGame* SaveGameInstance, ADiaController* Controller);
};
