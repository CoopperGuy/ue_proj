// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MonsterSpawnSubSystem.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "System/MonsterManager.h"

void UMonsterSpawnSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
    LoadMonsterSpawnGroupData();
}

void UMonsterSpawnSubSystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMonsterSpawnSubSystem::SpawnMonsterGroup(FName GroupID, FVector CenterLocation, float Radius)
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;
	UMonsterManager* MM = GI->GetSubsystem<UMonsterManager>();
	if (!MM) return;

	if (MapSpawnCache.Contains(GroupID))
	{
		FMapSpawnInfo& SpawnInfo = MapSpawnCache[GroupID];
		int32 SpawnCount = FMath::RandRange(SpawnInfo.MinTotalSpawnCount, SpawnInfo.MaxTotalSpawnCount);
		TArray<FVector> SpawnLocations;
		SpawnLocations.Reserve(SpawnCount);
		for (int32 i = 0; i < SpawnCount;)
		{
			//유효한 장소를 찾으면 ++i
			FVector SpawnLocation;
			if (FindValidSpawnLocation(CenterLocation, Radius, SpawnLocation))
			{
				SpawnLocations.Add(SpawnLocation);
				++i;
			}
		}
		OptimizeSpawnDistribution(SpawnLocations);
		// 몬스터 스폰 로직 추가
		// 해당 몬스터를 랜덤한 위치에 차례대로 스폰
		// 아직 비중에 따른 스폰이라던가 이런것은 미구현 (차후 구현 예정)
		for (int32 i = 0; i < SpawnLocations.Num(); ++i)
		{
			const FVector& Location = SpawnLocations[i];
			const FMonsterSpawnInfo& SpawnMonsters = SpawnInfo.MonsterSpawnInfos[i % SpawnInfo.MonsterSpawnInfos.Num()];

			ADiaMonster* SpawnedMonster = MM->SpawnMonster(GetWorld(), SpawnMonsters.MonsterID, Location); // 임시 위치에 스폰
		}
	}
}

// 몬스터 스폰 로직 추가
//추가적인 상황에 대한 스폰 방식
void UMonsterSpawnSubSystem::TriggerEventSpawn(FName EventID)
{
}

void UMonsterSpawnSubSystem::UpdateDynamicSpawning(float DeltaTime)
{
}

void UMonsterSpawnSubSystem::StartWave(int32 WaveNumber)
{
}

void UMonsterSpawnSubSystem::EndCurrentWave()
{
}

bool UMonsterSpawnSubSystem::FindValidSpawnLocation(const FVector& Center, float Radius, FVector& OutLocation)
{
	FNavLocation NavLocation;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		if (NavSys->GetRandomPointInNavigableRadius(Center, Radius, NavLocation))
		{
			OutLocation = NavLocation.Location;
			return true;
		}
	}
	return false;
}

void UMonsterSpawnSubSystem::OptimizeSpawnDistribution(TArray<FVector>& SpawnLocations)
{
	// 스폰 위치 최적화 로직 추가
	for (int32 i = 0; i < SpawnLocations.Num(); ++i)
	{
		for (int32 j = i + 1; j < SpawnLocations.Num(); ++j)
		{
			if (FVector::Dist(SpawnLocations[i], SpawnLocations[j]) < 100.f) // 예시 거리
			{
				SpawnLocations.RemoveAt(j);
				--j;
			}
		}
	}
}

void UMonsterSpawnSubSystem::LoadMonsterSpawnGroupData()
{
    MapSpawnGroupData = LoadObject<UDataTable>(nullptr, *SpawnGroupDataPath);
    if (MapSpawnGroupData)
    {
        MapSpawnCache.Empty();
        TArray<FName> RowNames = MapSpawnGroupData->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            FMapSpawnInfo* MapGroupRow = MapSpawnGroupData->FindRow<FMapSpawnInfo>(RowName, TEXT(""));
            if (MapGroupRow)
            {
                MapSpawnCache.Emplace(MapGroupRow->GroupName, *MapGroupRow);
            }
        }
    }
#ifdef UE_BUILD_DEBUG
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UMonsterSpawnSubSystem: Failed to load item data table from path: %s"), *SpawnGroupDataPath);
    }
#endif
}
