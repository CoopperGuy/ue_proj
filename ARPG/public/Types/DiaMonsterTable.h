// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DiaDropTable.generated.h"

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
	int32 Attack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Defense;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Exp;

	//애니메이션 블루프린트
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UAnimBlueprint> AnimationBP;
	//매쉬
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> MonsterMesh;

	//BlackBorad
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UBlackboardData> BlackboardAsset;
	
	//BehaviorTree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;
};
