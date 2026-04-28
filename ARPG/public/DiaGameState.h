// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DataAsset/DiaRoomType.h"
#include "Types/DiaMonsterTable.h"
#include "DiaGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDiaRoomClearedDelegate, FGuid);
DECLARE_MULTICAST_DELEGATE_OneParam(FDiaRoomBattleStartDelegate, FGuid);
DECLARE_MULTICAST_DELEGATE_OneParam(FDiaStageClearedDelegate, FGuid);
/**
 * 
 */
UCLASS()
class ARPG_API ADiaGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:	
	void SpawnRoomMonsters(const FGuid& NewRoomID, const FVector& CenterPos, const ETileType NewRoomType, const ESpawnType SpawnType, const float TileSize);

	void ReportMonsterDeath(const FGuid& RoomID);
	void ClearCurrentLevel();
private:
	UPROPERTY()
	FGuid CurrentRoomID;

	UPROPERTY()
	ETileType CurrentRoomType = ETileType::Empty;

	UPROPERTY()
	int32 CurrentMaxMonsterCount = 0;

	UPROPERTY()
	bool bIsInBattle;

public:
	FDiaRoomClearedDelegate OnRoomCleared;
	FDiaRoomBattleStartDelegate OnRoomBattleStart;
	FDiaStageClearedDelegate OnStageCleared;
};
