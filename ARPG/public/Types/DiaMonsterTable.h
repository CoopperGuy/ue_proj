// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"

#include "Engine/DataTable.h"
#include "DiaMonsterTable.generated.h"

class UBlackboardData;
class UBehaviorTree;
class ADiaMonster;

USTRUCT(BlueprintType)
struct FMonsterInfo : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MonsterID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Level = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaxHP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaxMP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Attack = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRange = 150.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Defense = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Exp = 0;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	TArray<int32> MonsterSkills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	FGameplayTagContainer MonsterTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ADiaMonster> BP_MonsterAsset;
};


USTRUCT(BlueprintType)
struct FMonsterSpawnInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MonsterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 SpawnLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SpawnWeight = 0.0f;

};


USTRUCT(BlueprintType)
struct FMapSpawnInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MapID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	TArray<FMonsterSpawnInfo> MonsterSpawnInfos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MinTotalSpawnCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MaxTotalSpawnCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnInterval = 5.f;


};
