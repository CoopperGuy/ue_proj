// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/MapGenerate.h"
#include "Types/DiaMonsterTable.h"
#include "DiaRoomType.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaRoomType : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Info")
	FText RoomID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Info")
	TSoftClassPtr<AActor> Roomclass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Info")
	FIntPoint RoomSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Info")
	ETileType TileType = ETileType::Empty;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room Info")
	ESpawnType SpawnType = ESpawnType::Normal;

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("DiaMap", GetFName());
	}
};
