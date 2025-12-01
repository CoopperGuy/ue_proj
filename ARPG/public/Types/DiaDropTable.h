// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DiaDropTable.generated.h"

USTRUCT(BlueprintType)
struct FItemDropInfo : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	    float DropRate = 0.0f; // 0.0 ~ 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	    int32 MinQuantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	    int32 MaxQuantity = 0;
};

USTRUCT(BlueprintType)
struct FMonsterDropTable : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MonsterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FItemDropInfo> DropItems;
};
