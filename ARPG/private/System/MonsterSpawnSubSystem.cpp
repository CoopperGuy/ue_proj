// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MonsterSpawnSubSystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/OverlapResult.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"
#include "Monster/DiaMonster.h"
#include "System/MonsterManager.h"
#include "Logging/ARPGLogChannels.h"

namespace
{
	bool ShouldLogRetry(const int32 Retry)
	{
		return Retry <= 3 || (Retry % 5) == 0;
	}

	bool ShouldLogThrottled(UObject* ContextObject, TMap<TWeakObjectPtr<UObject>, double>& LastLogTimes, const double IntervalSeconds)
	{
		if (!IsValid(ContextObject))
		{
			return false;
		}

		const UWorld* World = ContextObject->GetWorld();
		if (!IsValid(World))
		{
			return true;
		}

		const double Now = World->GetTimeSeconds();
		double& LastLogTime = LastLogTimes.FindOrAdd(ContextObject);
		if ((Now - LastLogTime) < IntervalSeconds)
		{
			return false;
		}

		LastLogTime = Now;
		return true;
	}
}

void UMonsterSpawnSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
    LoadMonsterSpawnGroupData();
}

void UMonsterSpawnSubSystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMonsterSpawnSubSystem::SpawnMonsterGroup(FName GroupID, FVector CenterLocation, float Radius, FGuid RoomID)
{
	constexpr int32 MaxRetryCount = 9;
	UGameInstance* GI = GetWorld()->GetGameInstance();
    UMonsterManager* MM = GI ? GI->GetSubsystem<UMonsterManager>() : nullptr;
    const FMapSpawnInfo* Group = MapSpawnCache.Find(GroupID);
    if (!MM || !Group || Group->MonsterSpawnInfos.IsEmpty() || Group->MaxTotalSpawnCount <= 0)
    {
        OnMonsterGroupSpawned.ExecuteIfBound(TArray<ADiaMonster*>(), false);
        ResetRetry(GroupID);
        return;
    }
    FRandomStream Random(RoomID.IsValid() ? static_cast<int32>(GetTypeHash(RoomID)) : GetTypeHash(GroupID));

    // 내비게이션 시스템 준비 상태 확인: 준비 전이면 잠시 후 재시도
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys)
    {
        const int32 Retry = GetAndIncrementRetry(GroupID);
        const float Delay = FMath::Min(0.5f + Retry * 0.1f, 3.0f);
		if (Retry > MaxRetryCount)
		{
			TArray<ADiaMonster*> SpawnedMonsters;
			OnMonsterGroupSpawned.ExecuteIfBound(SpawnedMonsters, false);
			ResetRetry(GroupID);
			return;
		}
		if (UWorld* World = GetWorld())
        {
            FTimerHandle Handle;
            World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &UMonsterSpawnSubSystem::SpawnMonsterGroup, GroupID, CenterLocation, Radius, RoomID), Delay, false);
        }
        return;
    }

    const bool bNavBuilding = NavSys->IsNavigationBeingBuilt(GetWorld());
    ANavigationData* DefaultNav = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
    if (bNavBuilding || DefaultNav == nullptr)
    {
        const int32 Retry = GetAndIncrementRetry(GroupID);
        const float Delay = FMath::Min(0.5f + Retry * 0.1f, 3.0f);
		if (Retry > MaxRetryCount)
		{
			TArray<ADiaMonster*> SpawnedMonsters;
			OnMonsterGroupSpawned.ExecuteIfBound(SpawnedMonsters, false);
			ResetRetry(GroupID);
			return;
		}
        if (UWorld* World = GetWorld())
        {
            FTimerHandle Handle;
            World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &UMonsterSpawnSubSystem::SpawnMonsterGroup, GroupID, CenterLocation, Radius, RoomID), Delay, false);
        }
        return;
    }

	if (MapSpawnCache.Contains(GroupID))
	{
		FMapSpawnInfo& SpawnInfo = MapSpawnCache[GroupID];
        const int32 MinCount = FMath::Max(1, SpawnInfo.MinTotalSpawnCount);
        const int32 SpawnCount = FMath::Clamp(Random.RandRange(MinCount, FMath::Max(MinCount, SpawnInfo.MaxTotalSpawnCount)), 1, 64);
        TArray<FVector> SpawnLocations;
        EDiaEncounterPattern Pattern = SpawnInfo.EncounterPattern;
        if (Pattern == EDiaEncounterPattern::Automatic)
            Pattern = static_cast<EDiaEncounterPattern>(Random.RandRange(1, 3));
        BuildEncounterLocations(CenterLocation, Radius, SpawnCount, Pattern, Random, SpawnLocations);
        if (SpawnLocations.Num() == 0)
        {
            const int32 Retry = GetAndIncrementRetry(GroupID);
            const float Delay = FMath::Min(0.5f + Retry * 0.1f, 3.0f);
			if (Retry > MaxRetryCount)
			{
				TArray<ADiaMonster*> SpawnedMonsters;
				OnMonsterGroupSpawned.ExecuteIfBound(SpawnedMonsters, false);
				ResetRetry(GroupID);
				return;
			}
            if (UWorld* World = GetWorld())
            {
                FTimerHandle Handle;
                World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateUObject(this, &UMonsterSpawnSubSystem::SpawnMonsterGroup, GroupID, CenterLocation, Radius, RoomID), Delay, false);
            }
            return;
        }
        if (SpawnLocations.Num() < SpawnCount)
            UE_LOG(LogARPG_Spawn, Warning, TEXT("Encounter %s fits %d/%d monsters; using validated positions only."), *GroupID.ToString(), SpawnLocations.Num(), SpawnCount);
        // Spawn only at validated formation slots, using authored monster weights.
		
		TArray<ADiaMonster*> SpawnedMonsters;
        for (int32 i = 0; i < SpawnLocations.Num(); ++i)
		{
			const FVector& Location = SpawnLocations[i];
            float TotalWeight = 0.f;
            for (const FMonsterSpawnInfo& Entry : SpawnInfo.MonsterSpawnInfos) TotalWeight += FMath::Max(0.f, Entry.SpawnWeight);
            int32 MonsterTypeIndex = Random.RandRange(0, SpawnInfo.MonsterSpawnInfos.Num() - 1);
            if (TotalWeight > 0.f)
            {
                float Roll = Random.FRand() * TotalWeight;
                for (int32 EntryIndex = 0; EntryIndex < SpawnInfo.MonsterSpawnInfos.Num(); ++EntryIndex)
                {
                    Roll -= FMath::Max(0.f, SpawnInfo.MonsterSpawnInfos[EntryIndex].SpawnWeight);
                    if (Roll < 0.f) { MonsterTypeIndex = EntryIndex; break; }
                }
            }
			const FMonsterSpawnInfo& SpawnMonsters = SpawnInfo.MonsterSpawnInfos[MonsterTypeIndex];

            ADiaMonster* SpawnedMonster = MM->SpawnMonster(GetWorld(), SpawnMonsters.MonsterID, Location); 
            if (!IsValid(SpawnedMonster))
            {
                UE_LOG(LogARPG_Spawn, Error, TEXT("[Spawn] Failed to spawn monster id=%s at %s"), *SpawnMonsters.MonsterID.ToString(), *Location.ToString());
            }
            if (IsValid(SpawnedMonster)) SpawnedMonsters.Add(SpawnedMonster);
		}
        const bool bComplete = SpawnedMonsters.Num() == SpawnLocations.Num();
        if (!bComplete)
        {
            for (ADiaMonster* Monster : SpawnedMonsters) MM->DespawnMonster(Monster);
            SpawnedMonsters.Reset();
        }
        OnMonsterGroupSpawned.ExecuteIfBound(SpawnedMonsters, bComplete);
        ResetRetry(GroupID);
	}
}

void UMonsterSpawnSubSystem::DeSpawnMonsterGroup(ADiaMonster* Monster)
{
	UMonsterManager* MM = GetWorld()->GetGameInstance()->GetSubsystem<UMonsterManager>();
	if (!MM) return;

	MM->DespawnMonster(Monster);
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

TArray<FMapSpawnInfo> UMonsterSpawnSubSystem::GetSpawnInfosForMap(FName MapID) const
{
	TArray<FMapSpawnInfo> Result;

	for(const auto& Pair : MapSpawnCache)
	{
		if (Pair.Value.MapID == MapID)
		{
			Result.Emplace(Pair.Value);
		}
	}

	return Result;
}

TArray<FMapSpawnInfo> UMonsterSpawnSubSystem::GetSpawnInfosForMapWithSpawnType(FName MapID, ESpawnType SpawnType) const
{
	TArray<FMapSpawnInfo> Result;

	for (const auto& Pair : MapSpawnCache)
	{
		if (Pair.Value.MapID == MapID && Pair.Value.SpawnType == SpawnType)
		{
			Result.Emplace(Pair.Value);
		}
	}

	return Result;
}

bool UMonsterSpawnSubSystem::FindValidSpawnLocation(const FVector& Center, float Radius, FVector& OutLocation)
{
	FNavLocation NavLocation;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		static TMap<TWeakObjectPtr<UObject>, double> LastLogTimes;
		if (ShouldLogThrottled(this, LastLogTimes, 1.0))
		{
			UE_LOG(LogARPG_Spawn, Verbose, TEXT("[Spawn] FindValidSpawnLocation: NavSys null"));
		}
		return false;
	}

	ANavigationData* DefaultNav = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
	if (!DefaultNav)
	{
		static TMap<TWeakObjectPtr<UObject>, double> LastLogTimes;
		if (ShouldLogThrottled(this, LastLogTimes, 1.0))
		{
			UE_LOG(LogARPG_Spawn, Verbose, TEXT("[Spawn] FindValidSpawnLocation: DefaultNav is null"));
		}
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
				static TMap<TWeakObjectPtr<UObject>, double> LastLogTimes;
				if (ShouldLogThrottled(this, LastLogTimes, 1.0))
				{
					UE_LOG(LogARPG_Spawn, Verbose, TEXT("[Spawn] FindValidSpawnLocation: ProjectPointToNavigation failed. Center=%s"), *Center.ToString());
				}
			}
		}

	// 3) 최후 수단: 탐색 실패 로그
	static TMap<TWeakObjectPtr<UObject>, double> LastLogTimes;
	if (ShouldLogThrottled(this, LastLogTimes, 1.0))
	{
		UE_LOG(LogARPG_Spawn, Verbose, TEXT("[Spawn] FindValidSpawnLocation: failed. Center=%s Radius=%.1f"), *Center.ToString(), Radius);
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
		UE_LOG(LogARPG_Spawn, Warning, TEXT("UMonsterSpawnSubSystem: Failed to load item data table from path: %s"), *SpawnGroupDataPath);
	}
#endif
}

void UMonsterSpawnSubSystem::BuildEncounterLocations(const FVector& Center, float Radius, int32 Count,
    EDiaEncounterPattern Pattern, FRandomStream& Random, TArray<FVector>& OutLocations)
{
    UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!Nav) return;
    const APlayerController* PC = GetWorld()->GetFirstPlayerController();
    const APawn* Player = PC ? PC->GetPawn() : nullptr;
    const float SafeRadius = FMath::Max(0.f, Radius - 100.f);
    const float Rotation = Random.RandRange(0, 3) * 90.f;
    // Try the chosen formation first, then a bounded interior grid for obstacles.
    for (int32 Slot = 0; Slot < 128 && OutLocations.Num() < Count; ++Slot)
    {
        FVector Offset;
        if (Slot < 64 && Pattern == EDiaEncounterPattern::Ring)
        {
            const int32 Ring = Slot / 16;
            const float Angle = (Slot % 16) * 2.f * PI / 16.f;
            const float RingRadius = SafeRadius * (1.f - Ring * 0.23f);
            Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * RingRadius;
        }
        else if (Slot < 64 && Pattern == EDiaEncounterPattern::TwoFlanks)
        {
            const int32 Row = (Slot / 2) % 8;
            const float X = (Slot % 2 == 0 ? -1.f : 1.f) * SafeRadius * (0.8f - (Slot / 16) * 0.18f);
            Offset = FVector(X, SafeRadius * (-0.85f + Row * 1.7f / 7.f), 0.f);
        }
        else
        {
            const int32 Cell = Slot % 64;
            Offset = FVector(-1.f + (Cell % 8) * 2.f / 7.f, -1.f + (Cell / 8) * 2.f / 7.f, 0.f) * SafeRadius;
        }
        const FVector Desired = Center + Offset.RotateAngleAxis(Rotation, FVector::UpVector);
        FNavLocation Projected;
        if (!Nav->ProjectPointToNavigation(Desired, Projected, FVector(70.f, 70.f, 250.f))) continue;
        const FVector Position = Projected.Location;
        if (FMath::Abs(Position.X - Center.X) > SafeRadius || FMath::Abs(Position.Y - Center.Y) > SafeRadius) continue;
        if (Player && FVector::DistSquared2D(Position, Player->GetActorLocation()) < FMath::Square(250.f)) continue;
        if (OutLocations.ContainsByPredicate([&Position](const FVector& Other) { return FVector::DistSquared2D(Position, Other) < FMath::Square(110.f); })) continue;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(EncounterSpawn), false);
        if (GetWorld()->OverlapBlockingTestByChannel(Position + FVector(0.f, 0.f, 95.f), FQuat::Identity,
            ECC_Pawn, FCollisionShape::MakeCapsule(42.f, 88.f), Params)) continue;
        OutLocations.Add(Position);
    }
}
