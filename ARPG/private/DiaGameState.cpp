// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaGameState.h"
#include "Monster/DiaMonster.h"
#include "System/MapInfoSubsystem.h"
#include "System/MonsterSpawnSubSystem.h"

bool ADiaGameState::SpawnRoomMonsters(const FGuid& NewRoomID, const FVector& CenterPos, const ETileType NewRoomType, const ESpawnType SpawnType, const float TileSize)
{
	if (!NewRoomID.IsValid() || ClearedRoomIDs.Contains(NewRoomID) || bIsInBattle)
	{
		return false;
	}

	const UMapInfoSubsystem* MapInfoSubsystem = GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UMapInfoSubsystem>() : nullptr;
	UMonsterSpawnSubSystem* MonsterSpawnSubSystem = GetWorld()->GetSubsystem<UMonsterSpawnSubSystem>();

	if (!IsValid(MapInfoSubsystem) || !IsValid(MonsterSpawnSubSystem))
	{
		return false;
	}

	FName MapID = MapInfoSubsystem->GetCurrentMapID();
	FName SpawnGroupName = NAME_None;
	const TArray<FMapSpawnInfo>& SpawnInfo = MonsterSpawnSubSystem->GetSpawnInfosForMapWithSpawnType(MapID, SpawnType);
	if (SpawnInfo.Num() > 0)
	{
		const int32 RandSpawnInfoIndex = FMath::RandRange(0, SpawnInfo.Num() - 1);
		SpawnGroupName = SpawnInfo[RandSpawnInfoIndex].GroupName;
	}

	if (SpawnGroupName == NAME_None)
	{
		return false;
	}

	CurrentRoomID = NewRoomID;
	CurrentRoomType = NewRoomType;
	CurrentMaxMonsterCount = 0;
	bIsInBattle = true;

	const FGuid SpawnRoomID = NewRoomID;
	MonsterSpawnSubSystem->OnMonsterGroupSpawned.BindLambda([this, SpawnRoomID](const TArray<ADiaMonster*>& InSpawnedMonsters)
	{
		if (CurrentRoomID != SpawnRoomID)
		{
			return;
		}

		int32 ValidMonsterCount = 0;
		for (ADiaMonster* Monster : InSpawnedMonsters)
		{
			if (Monster)
			{
				Monster->SetOwningRoom(SpawnRoomID);
				++ValidMonsterCount;
			}
		}

		CurrentMaxMonsterCount = ValidMonsterCount;
		if (CurrentMaxMonsterCount <= 0)
		{
			ClearedRoomIDs.Add(SpawnRoomID);
			OnRoomCleared.Broadcast(SpawnRoomID);
			bIsInBattle = false;
			CurrentRoomType = ETileType::Empty;
			CurrentRoomID.Invalidate();
			return;
		}

		OnRoomBattleStart.Broadcast(SpawnRoomID);
	});

	MonsterSpawnSubSystem->SpawnMonsterGroup(SpawnGroupName, CenterPos, TileSize);
	return true;
}

void ADiaGameState::ReportMonsterDeath(const FGuid& RoomID)
{
	if (CurrentRoomID != RoomID || !bIsInBattle)
	{
		return;
	}

	CurrentMaxMonsterCount = FMath::Max(0, CurrentMaxMonsterCount - 1);

	if (CurrentMaxMonsterCount == 0)
	{
		const FGuid ClearedRoomID = CurrentRoomID;
		ClearedRoomIDs.Add(ClearedRoomID);
		OnRoomCleared.Broadcast(ClearedRoomID);

		bIsInBattle = false;
		CurrentRoomType = ETileType::Empty;
		CurrentRoomID.Invalidate();
	}	
}

void ADiaGameState::ClearCurrentLevel()
{
	OnStageCleared.Broadcast(CurrentRoomID);
}
