// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Types/MapGenerate.h"
#include "Types/DiaMapTable.h"
#include "Types/DiaMonsterTable.h"
#include "DiaMapGeneratorSubsystem.generated.h"

class ADiaRoomBase;

USTRUCT(BlueprintType)
struct FDiaRoomData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RoomID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint RoomSize = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint RoomPosition = FIntPoint::ZeroValue; // 맵 내에서의 위치 (X, Y)

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAnchor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETileType TileType = ETileType::Empty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Directions = 0; // Derived world-facing direction mask for corridor visuals.

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RotateDegree = 0; // 0, 90, 180, 270.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDiaRoomPort> ActivePorts;
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
	UFUNCTION(BlueprintCallable, Category = "Map|Generation")
	void GenerateMapWithSize(FName MapID, int32 InMapWidth, int32 InMapHeight);
	UFUNCTION(BlueprintCallable, Category = "Map|Generation")
	void SetMapSize(int32 InMapWidth, int32 InMapHeight);
	UFUNCTION(BlueprintPure, Category = "Map|Generation")
	FIntPoint GetMapSize() const { return FIntPoint(MapWidth, MapHeight); }
	UFUNCTION(BlueprintPure, Category = "Map|Generation")
	bool TryGetStartRoomWorldLocation(FVector& OutLocation, float ZOffset = 100.f) const;
	void PrintMapAsText() const;
	void SaveMapToFile(const FString& FilePath) const;
	
	int32 GetIndex(int32 X, int32 Y) const { return Y * MapWidth + X; }
	void GetIndexToCoord(int32 Index, int32& OutX, int32& OutY) const { OutX = Index % MapWidth; OutY = Index / MapWidth; }
	
	FDiaAdjacencyRule FindAdjacencyRule(const FName& RoomID) const;

protected:
	bool ShouldGenerateMapForWorld(const UWorld& World) const;
	void FindRoomCandidates(const FDiaAdjacencyRule& Rule, TArray<FDiaAdjacencyRule>& OutCandidates, int32 CurrentRooms) const;
	void LoadRoomData(const FName& RoomID);
	void NormalizeMapSettings();
	void RandomizeMapSize();
	FIntPoint ChooseStartRoomPosition(const FIntPoint& RoomSize) const;
	bool CanPlaceRoom(int32 X, int32 Y) const;
	bool CanPlaceRoom(const FIntPoint& Position, const FIntPoint& RoomSize) const;
	void PlaceRoomData(const FDiaRoomData& RoomData);
	TArray<FDiaRoomPort> GetRoomPorts(const UDiaRoomType* RoomType) const;
	bool TryBuildConnectedRoomData(const FDiaRoomData& SourceRoom, const FDiaRoomPort& SourcePort, const FDiaAdjacencyRule& CandidateRule, FDiaRoomData& OutRoomData, FDiaRoomPort& OutCandidatePort) const;
	void AddActivePortToPlacedRoom(const FIntPoint& AnchorPosition, const FDiaRoomPort& ActivePort);
	uint8 MakeWorldDirectionsFromActivePorts(const FDiaRoomData& RoomData) const;
	bool IsActivePortLinked(const FDiaRoomData& RoomData, const FDiaRoomPort& ActivePort) const;
	void UpdatePlacedRoomData(const FDiaRoomData& RoomData);
	void ValidateAndRepairActivePortLinks();

	void BFSGenerateMap(FName MapID);
	bool TryPlaceEndRoom(int32 CurrentRooms);
	void CreateMapFromData();
	ADiaRoomBase* CreateRoomActor(UDiaRoomType* RoomType, const FDiaRoomData& RoomData, float TileSize);
	void MovePlayerToStartRoom();

	void CalcuateCorridorType(uint8 Directions, int32& OutRotateDegree, FName& OutRoomID) const;
	void CalcuateCorridorDegree(uint8 SourceDirections, uint8 DestDirections, int32& OutRotateDegree) const;
protected:
	UPROPERTY()
	TArray<FDiaAdjacencyRule> AdjacencyRules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TArray<FDiaRoomData> MapData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TMap<FGuid,TObjectPtr<ADiaRoomBase>> MapObjList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation", meta = (ClampMin = "1", ClampMax = "50", UIMin = "3", UIMax = "15"))
	int32 MapWidth = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation", meta = (ClampMin = "1", ClampMax = "50", UIMin = "3", UIMax = "15"))
	int32 MapHeight = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation")
	bool bRandomizeMapSizeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation", meta = (ClampMin = "1", ClampMax = "50", UIMin = "3", UIMax = "15"))
	int32 MinRandomMapWidth = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation", meta = (ClampMin = "1", ClampMax = "50", UIMin = "3", UIMax = "15"))
	int32 MaxRandomMapWidth = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation", meta = (ClampMin = "1", ClampMax = "50", UIMin = "3", UIMax = "15"))
	int32 MinRandomMapHeight = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation", meta = (ClampMin = "1", ClampMax = "50", UIMin = "3", UIMax = "15"))
	int32 MaxRandomMapHeight = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation", meta = (ClampMin = "1", UIMin = "1", UIMax = "20"))
	int32 MinMapRoomCount = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Generation", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.1", UIMax = "1.0"))
	float TargetFillRatio = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSoftObjectPtr<UDataTable> MapAdjacencyTable;

	UPROPERTY()
	TMap<FName, UDiaRoomType*> RoomDataCache;

	UPROPERTY()
	FDiaRoomData StartRoomData;

	UPROPERTY()
	TObjectPtr<ADiaRoomBase> StartRoomActor = nullptr;

	UPROPERTY()
	bool bHasStartRoomData = false;

public:
	TObjectPtr<ADiaRoomBase> GetRoomActor(const FGuid& RoomGuid) const;
	TObjectPtr<ADiaRoomBase> GetStartRoomActor() const { return StartRoomActor; }
};
