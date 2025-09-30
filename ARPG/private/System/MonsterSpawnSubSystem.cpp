// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MonsterSpawnSubSystem.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
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

    // 내비게이션 시스템 준비 상태 확인: 준비 전이면 잠시 후 재시도
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        const int32 Retry = GetAndIncrementRetry(GroupID);
        UE_LOG(LogTemp, Verbose, TEXT("[Spawn] NavSys null. Retry=%d Group=%s"), Retry, *GroupID.ToString());
        const float Delay = FMath::Min(0.5f + Retry * 0.1f, 3.0f);
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UMonsterSpawnSubSystem::SpawnMonsterGroup, GroupID, CenterLocation, Radius));
        }
        return;
    }

    const bool bNavBuilding = NavSys->IsNavigationBeingBuilt(GetWorld());
    ANavigationData* DefaultNav = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
    if (bNavBuilding || DefaultNav == nullptr)
    {
        const int32 Retry = GetAndIncrementRetry(GroupID);
        UE_LOG(LogTemp, Verbose, TEXT("[Spawn] Nav building or no default nav. Retry=%d Group=%s"), Retry, *GroupID.ToString());
        const float Delay = FMath::Min(0.5f + Retry * 0.1f, 3.0f);
        if (UWorld* World = GetWorld())
        {
            FTimerHandle Handle;
            World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &UMonsterSpawnSubSystem::SpawnMonsterGroup, GroupID, CenterLocation, Radius), Delay, false);
        }
        return;
    }

	if (MapSpawnCache.Contains(GroupID))
	{
		FMapSpawnInfo& SpawnInfo = MapSpawnCache[GroupID];
		int32 SpawnCount = FMath::RandRange(SpawnInfo.MinTotalSpawnCount, SpawnInfo.MaxTotalSpawnCount);
		TArray<FVector> SpawnLocations;
		SpawnLocations.Reserve(SpawnCount);
        int32 attempts = 0;
        const int32 maxAttempts = FMath::Max(SpawnCount * 10, 100);
        for (int32 i = 0; i < SpawnCount && attempts < maxAttempts;)
		{
			//유효한 장소를 찾으면 ++i
			FVector SpawnLocation;
            attempts++;
            if (FindValidSpawnLocation(CenterLocation, Radius, SpawnLocation))
			{
				SpawnLocations.Add(SpawnLocation);
				++i;
			}
		}
        if (SpawnLocations.Num() == 0)
        {
            const int32 Retry = GetAndIncrementRetry(GroupID);
            UE_LOG(LogTemp, Warning, TEXT("[Spawn] No valid locations. Retry=%d Group=%s Center=%s Radius=%.1f"), Retry, *GroupID.ToString(), *CenterLocation.ToString(), Radius);
            const float Delay = FMath::Min(0.5f + Retry * 0.1f, 3.0f);
            if (UWorld* World = GetWorld())
            {
                FTimerHandle Handle;
                World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &UMonsterSpawnSubSystem::SpawnMonsterGroup, GroupID, CenterLocation, Radius), Delay, false);
            }
            return;
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
            if (!IsValid(SpawnedMonster))
            {
                UE_LOG(LogTemp, Error, TEXT("[Spawn] Failed to spawn monster id=%s at %s"), *SpawnMonsters.MonsterID.ToString(), *Location.ToString());
            }
		}
        ResetRetry(GroupID);
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
	if (!NavSys)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Spawn] FindValidSpawnLocation: NavSys null"));
		return false;
	}

	ANavigationData* DefaultNav = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	if (!DefaultNav)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[Spawn] FindValidSpawnLocation: DefaultNav is null"));
		return false;
	}

	// 1) 먼저 도달 가능 지점(Reachable) 탐색을 큰 반경부터 작은 반경으로 시도
	const float Radii[4] = { Radius, Radius * 0.75f, Radius * 0.5f, Radius * 0.25f };
	for (float R : Radii)
	{
		if (R <= 10.f) continue; // 너무 작으면 스킵
		if (NavSys->GetRandomReachablePointInRadius(Center, R, NavLocation))
		{
			OutLocation = NavLocation.Location;
			return true;
		}
	}

	// 2) Center가 NavMesh 밖일 수 있으므로, 먼저 NavMesh로 투영 후 다시 시도
	{
		FNavLocation Projected;
		const FVector QueryExtent(500.f, 500.f, 2000.f);
		if (NavSys->ProjectPointToNavigation(Center, Projected, QueryExtent, DefaultNav, nullptr))
		{
			for (float R : Radii)
			{
				if (R <= 10.f) continue;
				if (NavSys->GetRandomReachablePointInRadius(Projected.Location, R, NavLocation))
				{
					OutLocation = NavLocation.Location;
					return true;
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Verbose, TEXT("[Spawn] FindValidSpawnLocation: ProjectPointToNavigation failed. Center=%s"), *Center.ToString());
		}
	}

	// 3) 최후 수단: 탐색 실패 로그
	UE_LOG(LogTemp, Warning, TEXT("[Spawn] FindValidSpawnLocation: failed. Center=%s Radius=%.1f"), *Center.ToString(), Radius);
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
