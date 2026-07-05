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
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool SaveGame(const FString& SlotName);
	bool LoadGame(const FString& SlotName);
	bool DoesSaveExist(const FString& SlotName) const;
	void DeleteSave(const FString& SlotName);	

	void FillSaveDataByGame(UDiaSaveGame* SaveGameInstance, const ADiaController* Controller);
	void ApplySaveDataToGame(const UDiaSaveGame* SaveGameInstance, ADiaController* Controller);

private:
	void OnPostLoadMapWithWorld(UWorld* InWorld);
	void ApplyPendingSaveData(UWorld* InWorld);
	FString NormalizeSavedLevelName(const FString& LevelName) const;

	UPROPERTY()
	TObjectPtr<UDiaSaveGame> PendingLoadedGame;

	int32 PendingLoadApplyAttempts = 0;
};
