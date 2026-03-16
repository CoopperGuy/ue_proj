// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaGameState.h"
#include "Monster/DiaMonster.h"
#include "System/MapInfoSubsystem.h"
#include "System/MonsterSpawnSubSystem.h"

void ADiaGameState::SpawnRoomMonsters(const FGuid& NewRoomID, const FVector& CenterPos, const float TileSize)
{
	CurrentRoomID = NewRoomID;

	const UMapInfoSubsystem* MapInfoSubsystem = GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UMapInfoSubsystem>() : nullptr;
	UMonsterSpawnSubSystem* MonsterSpawnSubSystem = GetWorld()->GetSubsystem<UMonsterSpawnSubSystem>();

	if (!IsValid(MapInfoSubsystem) || !IsValid(MonsterSpawnSubSystem))
	{
		return;
	}


	MonsterSpawnSubSystem->OnMonsterGroupSpawned.BindLambda([this](const TArray<ADiaMonster*>& InSpawnedMonsters)
	{
		this->CurrentMaxMonsterCount = InSpawnedMonsters.Num();
		this->bIsInBattle = true;

		for (ADiaMonster* Monster : InSpawnedMonsters)
		{
			if (Monster)
			{
				Monster->SetOwningRoom(this->CurrentRoomID);
			}
		}

		OnRoomBattleStart.Broadcast(CurrentRoomID);
	});

	FName MapID = MapInfoSubsystem->GetCurrentMapID();
	FName SpawnGroupName = NAME_None;
	const TArray<FMapSpawnInfo>& SpawnInfo = MonsterSpawnSubSystem->GetSpawnInfosForMap(MapID);
	if (SpawnInfo.Num() > 0)
	{
		const int32 RandSpawnInfoIndex = FMath::RandRange(0, SpawnInfo.Num() - 1);
		SpawnGroupName = SpawnInfo[RandSpawnInfoIndex].GroupName;
	}

	MonsterSpawnSubSystem->SpawnMonsterGroup(SpawnGroupName, CenterPos, TileSize);
}

void ADiaGameState::ReportMonsterDeath(const FGuid& RoomID)
{
	if (CurrentRoomID != RoomID)
	{
		return;
	}

	CurrentMaxMonsterCount = FMath::Max(0, CurrentMaxMonsterCount - 1);

	if (CurrentMaxMonsterCount == 0)
	{
		bIsInBattle = false;
		OnRoomCleared.Broadcast(CurrentRoomID);
	}
}

void ADiaGameState::ClearCurrentLevel()
{
}
