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


	void BFSGenerateMap(FName MapID);
	void CreateMapFromData();
	void CraeteRoomActor(UDiaRoomType* RoomType, const FIntPoint& RoomPosition, float TileSize);
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
