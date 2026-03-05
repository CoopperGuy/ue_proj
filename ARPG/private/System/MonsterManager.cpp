// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MonsterManager.h"
#include "System/MonsterPoolSubsystem.h"
#include "System/DiaMapGeneratorSubsystem.h"

#include "Map/DiaRoomBase.h"

#include "Monster/DiaMonster.h"

void UMonsterManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadMonsterData();
}

void UMonsterManager::Deinitialize()
{
	Super::Deinitialize();

    MonDataTable = nullptr;
    MonsterCache.Empty();
}

void UMonsterManager::LoadMonsterData()
{
    MonDataTable = LoadObject<UDataTable>(nullptr, *MonDataTablePath);
    if (MonDataTable)
    {
        MonsterCache.Empty();
        TArray<FName> RowNames = MonDataTable->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            FMonsterInfo* MonsterRow = MonDataTable->FindRow<FMonsterInfo>(RowName, TEXT(""));
            if (MonsterRow)
            {
                MonsterCache.Emplace(MonsterRow->MonsterID, *MonsterRow);
            }
        }
    }
#ifdef UE_BUILD_DEBUG
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UMonsterManager: Failed to load item data table from path: %s"), *MonDataTablePath);
    }
#endif
}

ADiaMonster* UMonsterManager::SpawnMonster(UWorld* World, FName MonsterID, const FVector& Location)
{
    // WorldSubsystem에서 풀 가져오기
    UMonsterPoolSubsystem* PoolSystem = World->GetSubsystem<UMonsterPoolSubsystem>();
    if (!PoolSystem)
        return nullptr;

    return PoolSystem->AcquireMonster(MonsterID, Location);
}

void UMonsterManager::DespawnMonster(ADiaMonster* Monster)
{
    if (!IsValid(Monster))
        return;

    UWorld* World = Monster->GetWorld();
    if (UMonsterPoolSubsystem* PoolSystem = World->GetSubsystem<UMonsterPoolSubsystem>())
    {
        PoolSystem->ReturnMonsterToPool(Monster);
    }
}

const FMonsterInfo* UMonsterManager::GetMonsterInfo(FName MonsterID) const
{
	if (const FMonsterInfo* MonsterInfo = MonsterCache.Find(MonsterID))
	{
		return MonsterInfo;
	}
    return nullptr;
}

void UMonsterManager::SetSpawnedMonstersForRoom(const FGuid& RoomID, const TArray<ADiaMonster*>& SpawnedMonsters)
{
	FRoomSpawnInfo NewSpawnInfo;
	NewSpawnInfo.RoomID = RoomID;
	NewSpawnInfo.SpawnedMonsters = SpawnedMonsters;

	RoomSpawnInfos.Add(NewSpawnInfo);
}

void UMonsterManager::ReportSpawnedMonsterDie(const FGuid& RoomID, ADiaMonster* DeadMonster)
{
    UWorld* CurrentWorld = GetGameInstance()->GetWorld();

    if (CurrentWorld)
    {
        UDiaMapGeneratorSubsystem* MyWorldSub = CurrentWorld->GetSubsystem<UDiaMapGeneratorSubsystem>();
        if (MyWorldSub)
        {
            ADiaRoomBase* RoomBase = MyWorldSub->GetRoomActor(RoomID);
            if (RoomBase)
            {
                RoomBase->RemoveRoomonster(DeadMonster);
            }
        }
    }

    for (FRoomSpawnInfo& SpawnInfo : RoomSpawnInfos)
    {
        if (SpawnInfo.RoomID == RoomID)
        {
            SpawnInfo.SpawnedMonsters.Remove(DeadMonster);

            if (SpawnInfo.SpawnedMonsters.Num() == 0)
            {
				SpawnInfo.SpawnedMonsters.Empty();
            }
            break;
        }
	}
}
