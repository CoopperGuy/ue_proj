// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DiaGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDiaRoomClearedDelegate, FGuid);
DECLARE_MULTICAST_DELEGATE_OneParam(FDiaRoomBattleStartDelegate, FGuid);
/**
 * 
 */
UCLASS()
class ARPG_API ADiaGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:	
	void SpawnRoomMonsters(const FGuid& NewRoomID, const FVector& CenterPos, const float TileSize);

	void ReportMonsterDeath(const FGuid& RoomID);
private:
	UPROPERTY()
	FGuid CurrentRoomID;

	UPROPERTY()
	int32 CurrentMaxMonsterCount = 0;

	UPROPERTY()
	bool bIsInBattle;

public:
	FDiaRoomClearedDelegate OnRoomCleared;
	FDiaRoomBattleStartDelegate OnRoomBattleStart;
};
