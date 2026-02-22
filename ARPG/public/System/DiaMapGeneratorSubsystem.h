// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/MapGenerate.h"
#include "DiaMapGeneratorSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FDiaRoomData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RoomID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint RoomSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint RoomPosition; // 맵 내에서의 위치 (X, Y)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETileType TileType = ETileType::Empty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Directions = 0; // 비트 필드로 방향 정보 저장 (예: 1<<0: North, 1<<1: East, 1<<2: South, 1<<3: West)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RotateDegree = 0; // 회전 각도 (0, 90, 180, 270) 그래서 int타입
};

class UDiaRoomType;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaMapGeneratorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
	void LoadAdjacencyRules();
	void GenerateMap(FName MapID);
	void PrintMapAsText() const;
	void SaveMapToFile(const FString& FilePath) const;
	
	int32 GetIndex(int32 X, int32 Y) const { return Y * MapWidth + X; }
	void GetIndexToCoord(int32 Index, int32& OutX, int32& OutY) const { OutX = Index % MapWidth; OutY = Index / MapWidth; }
	
	FDiaAdjacencyRule FindAdjacencyRule(const FName& RoomID) const;

protected:
	void FindRoomCandidates(const FDiaAdjacencyRule& Rule, TArray<FDiaAdjacencyRule>& OutCandidates) const;
	void LoadRoomData(const FName& RoomID);
	bool CanPlaceRoom(const FDiaAdjacencyRule& Rule, int32 X, int32 Y) const;
	EDiaDirection CanConnectRooms(const FDiaAdjacencyRule& SourceRule, const FDiaAdjacencyRule& DestRule) const;
	bool CanConnectRooms(const FDiaRoomData& SourceRoom, const FDiaRoomData& DestRoom, const EDiaDirection Direction, int32& OutRotateDegree) const;

	void BFSGenerateMap(FName MapID);
	void CreateMapFromData();
	void CraeteRoomActor(UDiaRoomType* RoomType, const FIntPoint& RoomPosition, float RotateDegree, float TileSize);
	void CheckConnectedPointCount(int32 X, int32 Y, uint8& OutDirections, FDiaRoomData& RoomData) const;

	void CalcuateCorridorType(uint8 Directions, int32& OutRotateDegree, FName& OutRoomID) const;
	void CalcuateCorridorDegree(uint8 SourceDirections, uint8 DestDirections, int32& OutRotateDegree) const;
protected:
	UPROPERTY()
	TArray<FDiaAdjacencyRule> AdjacencyRules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TArray<FDiaRoomData> MapData;

	int32 MapWidth = 5;
	int32 MapHeight = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSoftObjectPtr<UDataTable> MapAdjacencyTable;

	UPROPERTY()
	TMap<FName, UDiaRoomType*> RoomDataCache;

};
