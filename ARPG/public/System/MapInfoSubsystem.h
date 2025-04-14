// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/DiaMapTable.h"

#include "Subsystems/GameInstanceSubsystem.h"
#include "MapInfoSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UMapInfoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void CreateMapSpawnData();
private:
	// 맵 정보 로드
	void LoadMapInfo();

private:
	UPROPERTY()
	UDataTable* MapInfoDataTable;
	UPROPERTY()
	FString MapInfoDataTablePath = TEXT("/Game/Datatable/DT_MapInfoTable.DT_MapInfoTable");
	UPROPERTY()
	TMap<FName, FMapInfoTable> MapInfoCache;
	// 맵 정보 캐시
public:
	// 맵 정보 가져오기
	const TMap<FName, FMapInfoTable>& GetMapInfoCache() const { return MapInfoCache; }
	
	
};
