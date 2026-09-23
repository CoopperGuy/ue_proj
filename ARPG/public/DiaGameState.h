// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DataAsset/DiaRoomType.h"
#include "Types/DiaMonsterTable.h"
#include "DiaGameState.generated.h"

enum class EDiaRoomBattleState : uint8;

DECLARE_MULTICAST_DELEGATE_TwoParams(FDiaRoomStateChangeDelegate, FGuid, EDiaRoomBattleState);
DECLARE_MULTICAST_DELEGATE_OneParam(FDiaStageClearedDelegate, FGuid);
/**
 * 
 */
UCLASS()
class ARPG_API ADiaGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:	
	bool SpawnRoomMonsters(const FGuid& NewRoomID, const FVector& CenterPos, const ETileType NewRoomType, const ESpawnType SpawnType, const float TileSize);

	void ReportMonsterDeath(const FGuid& RoomID);
	void ClearCurrentLevel();
	bool IsRoomCleared(const FGuid& RoomID) const { return ClearedRoomIDs.Contains(RoomID); }
	bool IsInBattle() const { return bIsInBattle; }
    const TSet<FGuid>& GetClearedRoomIDs() const { return ClearedRoomIDs; }
    void RestoreClearedRooms(const TSet<FGuid>& Rooms) { ClearedRoomIDs = Rooms; }
private:
	UPROPERTY()
	FGuid CurrentRoomID;

	UPROPERTY()
	ETileType CurrentRoomType = ETileType::Empty;

	UPROPERTY()
	int32 CurrentMaxMonsterCount = 0;

	UPROPERTY()
	bool bIsInBattle = false;

	UPROPERTY()
	TSet<FGuid> ClearedRoomIDs;

public:
	FDiaRoomStateChangeDelegate OnRoonStateChange;
	FDiaStageClearedDelegate OnStageCleared;
};
