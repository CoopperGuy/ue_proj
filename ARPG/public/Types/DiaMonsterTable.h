// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DiaMonsterTable.generated.h"

class UBlackboardData;
class UBehaviorTree;

USTRUCT(BlueprintType)
struct FMonsterInfo : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MonsterID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxMP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Attack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRange = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Defense = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Exp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UAnimInstance> AnimationInstance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> MonsterMesh;

	//BlackBorad
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UBlackboardData> BlackboardAsset;
	
	//BehaviorTree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;
};


USTRUCT(BlueprintType)
struct FMonsterSpawnInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MonsterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpawnLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnWeight;

};


USTRUCT(BlueprintType)
struct FMapSpawnInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MapID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	TArray<FMonsterSpawnInfo> MonsterSpawnInfos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinTotalSpawnCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTotalSpawnCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnInterval = 5.f;


};
