// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Types/DiaMonsterTable.h"

#include "Subsystems/GameInstanceSubsystem.h"

#include "MonsterManager.generated.h"

class ADiaMonster;
/**
* 몬스터 데이터 관리 및 생성을 담당하는 서브시스템
* 해당 스펙 이상의 기능을 부여하지 말아야 한다.
 * 
 */
UCLASS()
class ARPG_API UMonsterManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

	void LoadMonsterData();

	// 몬스터 풀링
	ADiaMonster* SpawnMonster(UWorld* World, FName MonsterID, FVector Location);
    void DespawnMonster(ADiaMonster* Monster);

	const TMap<FName, FMonsterInfo>& GetMonsterCache() const { return MonsterCache; }
private:
	UPROPERTY()
    UDataTable* MonDataTable;
    
    UPROPERTY()
    FString MonDataTablePath = TEXT("/Game/Datatable/DT_DiaMonsterTable.DT_DiaMonsterTable");

    UPROPERTY()
    TMap<FName, FMonsterInfo> MonsterCache;
};
