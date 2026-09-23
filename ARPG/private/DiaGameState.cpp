// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaGameState.h"
#include "Map/DiaRoomBase.h"
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
	TArray<FMapSpawnInfo> SpawnInfo = MonsterSpawnSubSystem->GetSpawnInfosForMapWithSpawnType(MapID, SpawnType);
    SpawnInfo.Sort([](const FMapSpawnInfo& A, const FMapSpawnInfo& B) { return A.GroupName.LexicalLess(B.GroupName); });
	if (SpawnInfo.Num() > 0)
	{
		FRandomStream Random(static_cast<int32>(GetTypeHash(NewRoomID)));
		const int32 RandSpawnInfoIndex = Random.RandRange(0, SpawnInfo.Num() - 1);
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
	MonsterSpawnSubSystem->OnMonsterGroupSpawned.BindLambda([this, SpawnRoomID](const TArray<ADiaMonster*>& InSpawnedMonsters, bool isSuccess)
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
		if (!isSuccess || CurrentMaxMonsterCount <= 0)
		{
			//ClearedRoomIDs.Add(SpawnRoomID);
			OnRoonStateChange.Broadcast(SpawnRoomID, EDiaRoomBattleState::Idle);
			bIsInBattle = false;
			CurrentRoomType = ETileType::Empty;
			//CurrentRoomID.Invalidate();
			return;
		}

		OnRoonStateChange.Broadcast(SpawnRoomID, EDiaRoomBattleState::BattleActive);
	});

	MonsterSpawnSubSystem->SpawnMonsterGroup(SpawnGroupName, CenterPos, TileSize, NewRoomID);
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
		OnRoonStateChange.Broadcast(ClearedRoomID, EDiaRoomBattleState::Cleared);

		bIsInBattle = false;
		CurrentRoomType = ETileType::Empty;
		CurrentRoomID.Invalidate();
	}	
}

void ADiaGameState::ClearCurrentLevel()
{
	OnStageCleared.Broadcast(CurrentRoomID);
}
