// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/DiaMonsterTable.h"
#include "MonsterSpawnSubSystem.generated.h"

/**
 * 
 * 오직 스폰만을 관리하기 위해 만든 서브 시스템.
 * monstermanager는 몬스터의 데이터와 스폰 관리한다.
 * monsterpoolsubsystem은 몬스터의 풀링과 관리를 담당한다.
 */
UCLASS()
class ARPG_API UMonsterSpawnSubSystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
    // 특정 영역에 몬스터 그룹 스폰
    void SpawnMonsterGroup(FName GroupID, FVector CenterLocation, float Radius);

    // 이벤트 트리거 기반 스폰
    void TriggerEventSpawn(FName EventID);

    // 플레이어 주변 동적 스폰 관리
    void UpdateDynamicSpawning(float DeltaTime);

    // 웨이브 관리
    void StartWave(int32 WaveNumber);
    void EndCurrentWave();
    
private:
    // 네비메시에서 유효한 스폰 위치 찾기
    bool FindValidSpawnLocation(const FVector& Center, float Radius, FVector& OutLocation);

    // 스폰 위치 최적화
    void OptimizeSpawnDistribution(TArray<FVector>& SpawnLocations);
	
    void LoadMonsterSpawnGroupData();

    UPROPERTY()
    UDataTable* MapSpawnGroupData;
    
    UPROPERTY()
    FString SpawnGroupDataPath = TEXT("/Game/Datatable/DT_SpawnGroupTable.DT_SpawnGroupTable");

    UPROPERTY()
    TMap<FName, FMapSpawnInfo> MapSpawnCache;
};
