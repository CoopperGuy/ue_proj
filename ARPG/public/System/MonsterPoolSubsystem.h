// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/DiaDropTable.h"
#include "MonsterPoolSubsystem.generated.h"

class ADiaMonster;
class UMonsterManager;

// 풀 아이템 구조체
USTRUCT()
struct FPooledMonster
{
    GENERATED_BODY()
    
    UPROPERTY()
    ADiaMonster* Monster;
    
    UPROPERTY()
    bool bInUse;
    
    FDateTime LastUsedTime;

	FPooledMonster()
	{
		Monster = nullptr;
		bInUse = false;
		LastUsedTime = FDateTime::Now();
	}
};

USTRUCT()
struct FMonsterPoolController
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPooledMonster> MonsterPool;

	UPROPERTY()
	int32 ActiveCount;

    FDateTime LastPoolUsedTime;

	FMonsterPoolController()
	{
        ActiveCount = 0;
		MonsterPool.Reserve(5);
        LastPoolUsedTime = FDateTime::Now();

	}
};


/**
 * Init -> AcquireMonster -> PreparePool -> ExpandPool -> CreateNewMonster -> InitializeMonster -> ReturnMonsterToPool -> ResetMonsterState -> CleanupUnusedMonsters
 * 초기화 -> 몬스터 가져오기 -> 풀 준비 -> 확장 -> 생성->초기화->반환-> 상태 초기화 ->정리
 * 정리는 일정 시간마다 실행된다
 * 가져오는 함수는 AcquireMonster, 반환하는 함수는 ReturnMonsterToPool
 * 
 * 위의 순서 대로 초기화된다
 */
UCLASS()
class ARPG_API UMonsterPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
	/// 외부에서 호출 가능한 함수들
    // 풀에서 몬스터 가져오기
    ADiaMonster* AcquireMonster(FName MonsterID, const FVector& Location);
    
    // 풀에 몬스터 반환
    void ReturnMonsterToPool(ADiaMonster* Monster);

private:

   
    // 내부 함수들
    // 특정 타입의 풀 준비
    void PreparePool(FName MonsterID, int32 Count);
    // 풀 확장(초기 생성시 or 부족할 시)
    void ExpandPool(FName MonsterID, int32 Count);
    // 풀에 들어갈 몬스터 생성 (직접적으로 호출하지 않음)
    ADiaMonster* CreateNewMonster(FName MonsterID);
    // 생성한 몬스터 초기화
	void InitializeMonster(ADiaMonster* Monster, const FVector& Location, FName MonsterID);
    // 몬스터 상채 초기화
    void ResetMonsterState(ADiaMonster* Monster);
    // 사용하지 않는 몬스터와 풀 정리
    void CleanupUnusedMonsters();

    // 타입별 풀 관리
	UPROPERTY()
	TMap<FName, FMonsterPoolController> MonsterPools;
    
    UPROPERTY()
    TWeakObjectPtr<UMonsterManager> MonsterManager;

	UPROPERTY()
    int32 DefaultInitialPoolSize = 5;

    UPROPERTY()
    int32 MaxPoolSizePerType = 20;

    UPROPERTY()
    float PoolExpansionFactor = 1.5f;

	FTimerHandle CleanupTimerHandle;
	float CleanupInterval = 300.0f;
	const FVector InactiveLocation = FVector(0, 0, -10000); // 비활성화된 몬스터 위치
};
