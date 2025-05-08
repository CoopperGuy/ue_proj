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
    /**
     * 풀에서 몬스터를 가져오거나 새로 생성합니다.
     * @param MonsterID 스폰할 몬스터 ID
     * @param Location 스폰 위치
     * @return 생성된 몬스터 인스턴스
     */
    UFUNCTION(BlueprintCallable, Category = "Monster Pool")
    ADiaMonster* AcquireMonster(FName MonsterID, const FVector& Location);
    
    /**
     * 사용이 끝난 몬스터를 풀에 반환합니다.
     * @param Monster 반환할 몬스터 인스턴스
     */
    UFUNCTION(BlueprintCallable, Category = "Monster Pool")
    void ReturnMonsterToPool(ADiaMonster* Monster);

    /**
     * 디버깅용: 몬스터 풀 상태를 로그로 출력합니다.
     * @param MonsterID 확인할 몬스터 ID
     */
    UFUNCTION(BlueprintCallable, Category = "Monster Pool|Debug")
    void DebugMonsterPool(FName MonsterID);

private:

   
    // 내부 함수들
    /**
     * 지정된 몬스터 타입의 풀을 준비합니다.
     * @param MonsterID 몬스터 ID
     * @param Count 초기화할 몬스터 수
     */
    void PreparePool(FName MonsterID, int32 Count);
    /**
     * 몬스터 풀을 확장합니다.
     * @param MonsterID 확장할 몬스터 ID
     * @param Count 추가할 몬스터 수
     */
    void ExpandPool(FName MonsterID, int32 Count);
    /**
     * 풀에 추가할 새 몬스터를 생성합니다.
     * @param MonsterID 생성할 몬스터 ID
     * @return 생성된 몬스터 인스턴스
     */
    ADiaMonster* CreateNewMonster(FName MonsterID);
    /**
     * 몬스터를 초기화하고 활성화합니다.
     * @param Monster 초기화할 몬스터
     * @param Location 설정할 위치
     * @param MonsterID 몬스터 ID
     */
	void InitializeMonster(ADiaMonster* Monster, const FVector& Location, FName MonsterID);
    /**
     * 몬스터 상태를 초기화합니다.
     * @param Monster 초기화할 몬스터
     */
    void ResetMonsterState(ADiaMonster* Monster);
    /**
     * 미사용 몬스터를 주기적으로 정리합니다.
     */
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
