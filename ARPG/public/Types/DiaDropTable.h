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

	int32 Quantity = 1;
};

USTRUCT(BlueprintType)
struct FMonsterDropTable : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MonsterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FItemDropInfo> DropItems;

    TArray<FItemDropInfo> GetRandomizedDropItems() const
    {
        TArray<FItemDropInfo> RandomizedDrops;
        for (const FItemDropInfo& DropInfo : DropItems)
        {
            if (FMath::FRand() <= DropInfo.DropRate)
            {
                FItemDropInfo RandomizedDrop = DropInfo;
                const int32 MinDropQuantity = FMath::Min(DropInfo.MinQuantity, DropInfo.MaxQuantity);
                const int32 MaxDropQuantity = FMath::Max(DropInfo.MinQuantity, DropInfo.MaxQuantity);
                RandomizedDrop.Quantity = FMath::RandRange(MinDropQuantity, MaxDropQuantity);
                RandomizedDrops.Add(RandomizedDrop);
            }
        }
        return RandomizedDrops;
	}
};
