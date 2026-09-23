// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DiaMapGeneratorSubsystem.h"
#include "System/DiaSaveSubsystem.h"
#include "System/DiaSaveGame.h"
#include "System/DiaRewardSubsystem.h"
#include "DiaGameState.h"
#include "Engine/GameInstance.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Types/MapGenerate.h"
#include "DataAsset/DiaRoomType.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

#include "Map/DiaRoomBase.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameMode/DungeonGameMode.h"
#include "System/MonsterSpawnSubSystem.h"
#include "Logging/ARPGLogChannels.h"
#include "TimerManager.h"

namespace
{
	FIntPoint GetDirectionOffset(EDiaDirection Direction)
	{
		switch (Direction)
		{
		case EDiaDirection::North:
			return FIntPoint(1, 0);
		case EDiaDirection::East:
			return FIntPoint(0, 1);
		case EDiaDirection::South:
			return FIntPoint(-1, 0);
		case EDiaDirection::West:
			return FIntPoint(0, -1);
		default:
			return FIntPoint(0, 0);
		}
	}

	FIntPoint GetSafeRoomSize(const FIntPoint& RoomSize)
	{
		return FIntPoint(FMath::Max(1, RoomSize.X), FMath::Max(1, RoomSize.Y));
	}

	FIntPoint GetRotatedRoomSize(const FIntPoint& RoomSize, int32 RotateDegree)
	{
		const FIntPoint SafeSize = GetSafeRoomSize(RoomSize);
		const int32 RotateSteps = ((FMath::RoundToInt(RotateDegree / 90.f) % 4) + 4) % 4;
		if (RotateSteps % 2 == 0)
		{
			return SafeSize;
		}

		return FIntPoint(SafeSize.Y, SafeSize.X);
	}

	FIntPoint GetAdjacentRoomPosition(const FDiaRoomData& SourceRoom, const FIntPoint& DestRoomSize, EDiaDirection Direction)
	{
		const FIntPoint SourceSize = GetSafeRoomSize(SourceRoom.RoomSize);
		const FIntPoint SafeDestSize = GetSafeRoomSize(DestRoomSize);
		const int32 AlignX = FMath::FloorToInt(static_cast<float>(SourceSize.X - SafeDestSize.X) * 0.5f);
		const int32 AlignY = FMath::FloorToInt(static_cast<float>(SourceSize.Y - SafeDestSize.Y) * 0.5f);

		switch (Direction)
		{
		case EDiaDirection::North:
			return FIntPoint(SourceRoom.RoomPosition.X + SourceSize.X, SourceRoom.RoomPosition.Y + AlignY);
		case EDiaDirection::South:
			return FIntPoint(SourceRoom.RoomPosition.X - SafeDestSize.X, SourceRoom.RoomPosition.Y + AlignY);
		case EDiaDirection::East:
			return FIntPoint(SourceRoom.RoomPosition.X + AlignX, SourceRoom.RoomPosition.Y + SourceSize.Y);
		case EDiaDirection::West:
			return FIntPoint(SourceRoom.RoomPosition.X + AlignX, SourceRoom.RoomPosition.Y - SafeDestSize.Y);
		default:
			return SourceRoom.RoomPosition;
		}
	}
}

void UDiaMapGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//?ш린??MonsterSpawnSubSystem??癒쇱? 珥덇린?뷀븳??
	//?ш린??媛?Room??Spawn??紐ъ뒪??洹몃９ ?뺣낫瑜?濡쒕뱶?????덈룄濡??섍린 ?꾪빐??
	Collection.InitializeDependency<UMonsterSpawnSubSystem>();

	Super::Initialize(Collection);

	NormalizeMapSettings();
	MapData.SetNum(MapWidth * MapHeight);
}

void UDiaMapGeneratorSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UDiaMapGeneratorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!ShouldGenerateMapForWorld(InWorld))
	{
		UE_LOG(LogARPG_Map, Verbose, TEXT("UDiaMapGeneratorSubsystem: Skipping map generation for world: %s"),
			*InWorld.GetName());
		return;
	}

	if (AdjacencyRules.IsEmpty())
	{
		LoadAdjacencyRules();
	}

    UDiaSaveSubsystem* Saves = InWorld.GetGameInstance()->GetSubsystem<UDiaSaveSubsystem>();
    UDiaRewardSubsystem* Rewards = InWorld.GetGameInstance()->GetSubsystem<UDiaRewardSubsystem>();
    const UDiaSaveGame* Saved = Saves ? Saves->GetPendingLoadedGame() : nullptr;
    const bool bRestore = Saved && Saved->SaveVersion >= 1 && Saved->bHasDungeonFloor && Saved->IsFloorSnapshotValid();
    if (Rewards) Rewards->ResetFloor();
    if (bRestore)
    {
        MapWidth = Saved->FloorSize.X;
        MapHeight = Saved->FloorSize.Y;
        MapData = Saved->RoomLayout;
        StartRoomData = Saved->StartRoom;
        bHasStartRoomData = true;
        if (Rewards) Rewards->RestoreFloor(Saved->RoomRewards);
        if (ADiaGameState* GS = InWorld.GetGameState<ADiaGameState>()) GS->RestoreClearedRooms(Saved->ClearedRooms);
    }
    else
    {
        if (bRandomizeMapSizeOnBeginPlay) RandomizeMapSize();
        GeneratePlayableFloor(TEXT("Entrance"));
    }
    SaveMapToFile(FPaths::ProjectSavedDir() / TEXT("LastGeneratedMap.txt"));
    CreateMapFromData();
    if (!bRestore)
    {
        MovePlayerToStartRoom();
        InWorld.GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UDiaMapGeneratorSubsystem::MovePlayerToStartRoom));
    }
    if (ADungeonGameMode* Mode = InWorld.GetAuthGameMode<ADungeonGameMode>())
        InWorld.GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(Mode, &ADungeonGameMode::InitializeRoomRewards));

}

bool UDiaMapGeneratorSubsystem::ShouldGenerateMapForWorld(const UWorld& World) const
{
	return World.GetAuthGameMode<ADungeonGameMode>() != nullptr;
}

void UDiaMapGeneratorSubsystem::LoadAdjacencyRules()
{
	if (!MapAdjacencyTable.IsValid())
	{
		// 湲곕낯 DataTable 寃쎈줈 ?ㅼ젙
		MapAdjacencyTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Datatable/DT_DiaAdjacencyRules.DT_DiaAdjacencyRules")));
	}

	UDataTable* DataTable = MapAdjacencyTable.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Failed to load DataTable at path: %s"),
			*MapAdjacencyTable.ToString());
		return;
	}

	// 紐⑤뱺 ??媛?몄삤湲?
	TArray<FName> RowNames = DataTable->GetRowNames();
	AdjacencyRules.Empty();
	RoomDataCache.Empty();

	for (const FName& RowName : RowNames)
	{
		FDiaAdjacencyRule* RowData = DataTable->FindRow<FDiaAdjacencyRule>(RowName, TEXT("UDiaMapGeneratorSubsystem"));
		if (RowData)
		{
			AdjacencyRules.Add(*RowData);
			if (!RoomDataCache.Contains(RowData->SourceRoomID))
			{
				LoadRoomData(RowData->SourceRoomID);
			}
			for (const FRoomWeight& Candidate : RowData->CandidateWeights)
			{
				if (!RoomDataCache.Contains(Candidate.RoomID))
				{
					LoadRoomData(Candidate.RoomID);
				}
			}
		}
		
	}
}

//Soruce RoomID濡?Map??Generate?섏옄.
void UDiaMapGeneratorSubsystem::GenerateMap(FName MapID)
{
	GeneratePlayableFloor(MapID);
}

void UDiaMapGeneratorSubsystem::GenerateMapWithSize(FName MapID, int32 InMapWidth, int32 InMapHeight)
{
	SetMapSize(InMapWidth, InMapHeight);
	GenerateMap(MapID);
}

void UDiaMapGeneratorSubsystem::SetMapSize(int32 InMapWidth, int32 InMapHeight)
{
	MapWidth = InMapWidth;
	MapHeight = InMapHeight;
	NormalizeMapSettings();

	MapData.Empty();
	MapData.SetNum(MapWidth * MapHeight);
	StartRoomData = FDiaRoomData();
	StartRoomActor = nullptr;
	bHasStartRoomData = false;
}

void UDiaMapGeneratorSubsystem::NormalizeMapSettings()
{
	MapWidth = FMath::Clamp(MapWidth, 1, 50);
	MapHeight = FMath::Clamp(MapHeight, 1, 50);

	MinRandomMapWidth = FMath::Clamp(MinRandomMapWidth, 1, 50);
	MaxRandomMapWidth = FMath::Clamp(MaxRandomMapWidth, 1, 50);
	MinRandomMapHeight = FMath::Clamp(MinRandomMapHeight, 1, 50);
	MaxRandomMapHeight = FMath::Clamp(MaxRandomMapHeight, 1, 50);

	if (MinRandomMapWidth > MaxRandomMapWidth)
	{
		const int32 PreviousMin = MinRandomMapWidth;
		MinRandomMapWidth = MaxRandomMapWidth;
		MaxRandomMapWidth = PreviousMin;
	}

	if (MinRandomMapHeight > MaxRandomMapHeight)
	{
		const int32 PreviousMin = MinRandomMapHeight;
		MinRandomMapHeight = MaxRandomMapHeight;
		MaxRandomMapHeight = PreviousMin;
	}

	const int32 MapSize = FMath::Max(1, MapWidth * MapHeight);
	MinMapRoomCount = FMath::Clamp(MinMapRoomCount, 1, MapSize);
	TargetFillRatio = FMath::Clamp(TargetFillRatio, 0.f, 1.f);
}

void UDiaMapGeneratorSubsystem::RandomizeMapSize()
{
	NormalizeMapSettings();

	const int32 RandomWidth = FMath::RandRange(MinRandomMapWidth, MaxRandomMapWidth);
	const int32 RandomHeight = FMath::RandRange(MinRandomMapHeight, MaxRandomMapHeight);
	SetMapSize(RandomWidth, RandomHeight);

	UE_LOG(LogARPG_Map, Log, TEXT("Randomized map size: %dx%d"), MapWidth, MapHeight);
}

FIntPoint UDiaMapGeneratorSubsystem::ChooseStartRoomPosition(const FIntPoint& RoomSize) const
{
	const FIntPoint SafeSize = GetSafeRoomSize(RoomSize);
	const int32 MaxX = MapWidth - SafeSize.X;
	const int32 MaxY = MapHeight - SafeSize.Y;
	if (MaxX < 0 || MaxY < 0)
	{
		return FIntPoint::ZeroValue;
	}

	return FIntPoint(FMath::RandRange(0, MaxX), FMath::RandRange(0, MaxY));
}

//?⑥닚??鍮?怨듦컙?몄? 泥댄겕.
bool UDiaMapGeneratorSubsystem::CanPlaceRoom(int32 NextX, int32 NextY) const
{
	return CanPlaceRoom(FIntPoint(NextX, NextY), FIntPoint(1, 1));
}

bool UDiaMapGeneratorSubsystem::CanPlaceRoom(const FIntPoint& Position, const FIntPoint& RoomSize) const
{
	const FIntPoint SafeSize = GetSafeRoomSize(RoomSize);
	for (int32 OffsetX = 0; OffsetX < SafeSize.X; ++OffsetX)
	{
		for (int32 OffsetY = 0; OffsetY < SafeSize.Y; ++OffsetY)
		{
			const int32 X = Position.X + OffsetX;
			const int32 Y = Position.Y + OffsetY;
			if (X < 0 || X >= MapWidth || Y < 0 || Y >= MapHeight)
			{
				return false;
			}

			if (MapData[GetIndex(X, Y)].RoomID != NAME_None)
			{
				return false;
			}
		}
	}

	return true;
}

void UDiaMapGeneratorSubsystem::PlaceRoomData(const FDiaRoomData& RoomData)
{
	const FIntPoint SafeSize = GetSafeRoomSize(RoomData.RoomSize);
	for (int32 OffsetX = 0; OffsetX < SafeSize.X; ++OffsetX)
	{
		for (int32 OffsetY = 0; OffsetY < SafeSize.Y; ++OffsetY)
		{
			const int32 X = RoomData.RoomPosition.X + OffsetX;
			const int32 Y = RoomData.RoomPosition.Y + OffsetY;
			if (X < 0 || X >= MapWidth || Y < 0 || Y >= MapHeight)
			{
				continue;
			}

			FDiaRoomData CellData = RoomData;
			CellData.bIsAnchor = OffsetX == 0 && OffsetY == 0;
			MapData[GetIndex(X, Y)] = CellData;
		}
	}
}

//?닿굔 洹몃깷 bool媛??섍꺼???섎뒗???쇰떒 ?쇨????좎?, 洹쇰뜲 紐???留뚰겮 ?뚯쟾?덈뒗吏??諛섑솚 ?댁빞??
TArray<FDiaRoomPort> UDiaMapGeneratorSubsystem::GetRoomPorts(const UDiaRoomType* RoomType) const
{
	if (!RoomType)
	{
		return DiaMapGenerator::MakeDefaultPorts(FIntPoint(1, 1));
	}

	const TArray<FDiaRoomPort> RawPorts = RoomType->Ports.IsEmpty()
		? DiaMapGenerator::MakeDefaultPorts(RoomType->RoomSize)
		: RoomType->Ports;

	TArray<FDiaRoomPort> ValidPorts;
	ValidPorts.Reserve(RawPorts.Num());
	for (const FDiaRoomPort& Port : RawPorts)
	{
		if (!DiaMapGenerator::IsPortOnBoundary(Port, RoomType->RoomSize))
		{
			UE_LOG(LogARPG_Map, Warning, TEXT("Ignoring invalid room port. Room=%s Port=%s LocalCell=(%d,%d) Direction=%d RoomSize=(%d,%d)"),
				*RoomType->GetName(),
				*Port.PortID.ToString(),
				Port.LocalCell.X,
				Port.LocalCell.Y,
				static_cast<int32>(Port.Direction),
				RoomType->RoomSize.X,
				RoomType->RoomSize.Y);
			continue;
		}

		ValidPorts.AddUnique(Port);
	}

	return ValidPorts;
}

bool UDiaMapGeneratorSubsystem::TryBuildConnectedRoomData(const FDiaRoomData& SourceRoom, const FDiaRoomPort& SourcePort, const FDiaAdjacencyRule& CandidateRule, FDiaRoomData& OutRoomData, FDiaRoomPort& OutCandidatePort) const
{
	const UDiaRoomType* SourceRoomType = RoomDataCache.FindRef(SourceRoom.RoomID);
	const UDiaRoomType* CandidateRoomType = RoomDataCache.FindRef(CandidateRule.SourceRoomID);
	if (!SourceRoomType || !CandidateRoomType)
	{
		return false;
	}

	const FDiaRoomPort SourceWorldPort = DiaMapGenerator::RotatePortDegree(SourcePort, SourceRoomType->RoomSize, SourceRoom.RotateDegree);
	const FIntPoint SourceWorldCell = SourceRoom.RoomPosition + SourceWorldPort.LocalCell;
	const FIntPoint TargetAdjacentCell = SourceWorldCell + DiaMapGenerator::GetDirectionOffset(SourceWorldPort.Direction);
	const TArray<FDiaRoomPort> CandidatePorts = GetRoomPorts(CandidateRoomType);
	const int32 RotationStart = FMath::RandRange(0, 3);

	for (int32 RotationIndex = 0; RotationIndex < 4; ++RotationIndex)
	{
		const int32 RotateDegree = ((RotationStart + RotationIndex) % 4) * 90;
		const FIntPoint RotatedRoomSize = DiaMapGenerator::GetRotatedRoomSize(CandidateRoomType->RoomSize, RotateDegree);
		const int32 PortStart = CandidatePorts.IsEmpty() ? 0 : FMath::RandRange(0, CandidatePorts.Num() - 1);

		for (int32 PortIndex = 0; PortIndex < CandidatePorts.Num(); ++PortIndex)
		{
			const FDiaRoomPort& CandidatePort = CandidatePorts[(PortStart + PortIndex) % CandidatePorts.Num()];
			const FDiaRoomPort CandidateWorldPort = DiaMapGenerator::RotatePortDegree(CandidatePort, CandidateRoomType->RoomSize, RotateDegree);
			if (CandidateWorldPort.Direction != GetOppositeDirection(SourceWorldPort.Direction))
			{
				continue;
			}

			const FIntPoint CandidatePosition = TargetAdjacentCell - CandidateWorldPort.LocalCell;
			if (!CanPlaceRoom(CandidatePosition, RotatedRoomSize))
			{
				continue;
			}

			OutRoomData = FDiaRoomData();
			OutRoomData.RoomID = CandidateRule.SourceRoomID;
			OutRoomData.RoomSize = RotatedRoomSize;
			OutRoomData.RoomPosition = CandidatePosition;
			OutRoomData.RotateDegree = RotateDegree;
			OutRoomData.TileType = CandidateRoomType->TileType;
			OutRoomData.ActivePorts.Add(CandidatePort);
			OutRoomData.Directions = DiaMapGenerator::MakeDirectionByPorts(OutRoomData.ActivePorts, CandidateRoomType->RoomSize, OutRoomData.RotateDegree);
			OutCandidatePort = CandidatePort;
			return true;
		}
	}

	return false;
}

void UDiaMapGeneratorSubsystem::AddActivePortToPlacedRoom(const FIntPoint& AnchorPosition, const FDiaRoomPort& ActivePort)
{
	for (FDiaRoomData& RoomData : MapData)
	{
		if (RoomData.RoomID == NAME_None || RoomData.RoomPosition != AnchorPosition)
		{
			continue;
		}

		if (!RoomData.ActivePorts.Contains(ActivePort))
		{
			RoomData.ActivePorts.Add(ActivePort);
		}
		RoomData.Directions = MakeWorldDirectionsFromActivePorts(RoomData);
	}
}

uint8 UDiaMapGeneratorSubsystem::MakeWorldDirectionsFromActivePorts(const FDiaRoomData& RoomData) const
{
	const UDiaRoomType* RoomType = RoomDataCache.FindRef(RoomData.RoomID);
	if (!RoomType)
	{
		return RoomData.Directions;
	}

	return DiaMapGenerator::MakeDirectionByPorts(RoomData.ActivePorts, RoomType->RoomSize, RoomData.RotateDegree);
}

bool UDiaMapGeneratorSubsystem::IsActivePortLinked(const FDiaRoomData& RoomData, const FDiaRoomPort& ActivePort) const
{
	const UDiaRoomType* RoomType = RoomDataCache.FindRef(RoomData.RoomID);
	if (!RoomType || !DiaMapGenerator::IsPortOnBoundary(ActivePort, RoomType->RoomSize))
	{
		return false;
	}

	const FDiaRoomPort WorldPort = DiaMapGenerator::RotatePortDegree(ActivePort, RoomType->RoomSize, RoomData.RotateDegree);
	const FIntPoint SourceCell = RoomData.RoomPosition + WorldPort.LocalCell;
	const FIntPoint TargetCell = SourceCell + DiaMapGenerator::GetDirectionOffset(WorldPort.Direction);
	if (TargetCell.X < 0 || TargetCell.X >= MapWidth || TargetCell.Y < 0 || TargetCell.Y >= MapHeight)
	{
		return false;
	}

	const FDiaRoomData& OtherRoom = MapData[GetIndex(TargetCell.X, TargetCell.Y)];
	if (OtherRoom.RoomID == NAME_None || OtherRoom.RoomPosition == RoomData.RoomPosition)
	{
		return false;
	}

	const UDiaRoomType* OtherRoomType = RoomDataCache.FindRef(OtherRoom.RoomID);
	if (!OtherRoomType)
	{
		return false;
	}

	const EDiaDirection RequiredDirection = GetOppositeDirection(WorldPort.Direction);
	for (const FDiaRoomPort& OtherActivePort : OtherRoom.ActivePorts)
	{
		if (!DiaMapGenerator::IsPortOnBoundary(OtherActivePort, OtherRoomType->RoomSize))
		{
			continue;
		}

		const FDiaRoomPort OtherWorldPort = DiaMapGenerator::RotatePortDegree(OtherActivePort, OtherRoomType->RoomSize, OtherRoom.RotateDegree);
		const FIntPoint OtherSourceCell = OtherRoom.RoomPosition + OtherWorldPort.LocalCell;
		if (OtherSourceCell == TargetCell && OtherWorldPort.Direction == RequiredDirection)
		{
			return true;
		}
	}

	return false;
}

void UDiaMapGeneratorSubsystem::UpdatePlacedRoomData(const FDiaRoomData& RoomData)
{
	for (FDiaRoomData& CellData : MapData)
	{
		if (CellData.RoomID == NAME_None || CellData.RoomPosition != RoomData.RoomPosition)
		{
			continue;
		}

		const bool bWasAnchor = CellData.bIsAnchor;
		CellData = RoomData;
		CellData.bIsAnchor = bWasAnchor;
	}
}

void UDiaMapGeneratorSubsystem::ValidateAndRepairActivePortLinks()
{
	TArray<FDiaRoomData> AnchorRooms;
	for (const FDiaRoomData& RoomData : MapData)
	{
		if (RoomData.RoomID != NAME_None && RoomData.bIsAnchor)
		{
			AnchorRooms.Add(RoomData);
		}
	}

	for (FDiaRoomData& RoomData : AnchorRooms)
	{
		const UDiaRoomType* RoomType = RoomDataCache.FindRef(RoomData.RoomID);
		if (!RoomType)
		{
			continue;
		}

		const TArray<FDiaRoomPort> PossiblePorts = GetRoomPorts(RoomType);
		TArray<FDiaRoomPort> RepairedActivePorts;
		RepairedActivePorts.Reserve(RoomData.ActivePorts.Num());

		for (const FDiaRoomPort& ActivePort : RoomData.ActivePorts)
		{
			if (!PossiblePorts.Contains(ActivePort))
			{
				UE_LOG(LogARPG_Map, Warning, TEXT("Closing active port not present in possible ports. Room=%s Port=%s"),
					*RoomData.RoomID.ToString(),
					*ActivePort.PortID.ToString());
				continue;
			}

			if (!IsActivePortLinked(RoomData, ActivePort))
			{
				UE_LOG(LogARPG_Map, Warning, TEXT("Closing unlinked active port. Room=%s Position=(%d,%d) Port=%s LocalCell=(%d,%d) Direction=%d"),
					*RoomData.RoomID.ToString(),
					RoomData.RoomPosition.X,
					RoomData.RoomPosition.Y,
					*ActivePort.PortID.ToString(),
					ActivePort.LocalCell.X,
					ActivePort.LocalCell.Y,
					static_cast<int32>(ActivePort.Direction));
				continue;
			}

			RepairedActivePorts.AddUnique(ActivePort);
		}

		if (RepairedActivePorts.Num() != RoomData.ActivePorts.Num())
		{
			RoomData.ActivePorts = RepairedActivePorts;
			RoomData.Directions = MakeWorldDirectionsFromActivePorts(RoomData);
			UpdatePlacedRoomData(RoomData);
		}
	}
}

void UDiaMapGeneratorSubsystem::BFSGenerateMap(FName MapID)
{
	NormalizeMapSettings();
	const int32 MapSize = MapWidth * MapHeight;
	const int32 MaxGeneratedRooms = FMath::Clamp(FMath::RoundToInt(MapSize * TargetFillRatio), MinMapRoomCount, MapSize);
	int32 PlacedRoomCount = 0;
	bool bEndPlaced = false;
	MapData.Empty();
	MapData.SetNum(MapSize);
	StartRoomData = FDiaRoomData();
	StartRoomActor = nullptr;
	bHasStartRoomData = false;

	TQueue<FDiaRoomData> RoomQueue;
	const FDiaAdjacencyRule& MainRoom = FindAdjacencyRule(MapID);
	if (MainRoom.SourceRoomID == NAME_None)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: No adjacency rule found for RoomID: %s"), *MapID.ToString());
		return;
	}

	UDiaRoomType* RoomType = RoomDataCache.FindRef(MainRoom.SourceRoomID);

	FDiaRoomData MainRoomData;
	MainRoomData.RoomID = MainRoom.SourceRoomID;
	MainRoomData.RoomSize = RoomType ? GetSafeRoomSize(RoomType->RoomSize) : FIntPoint(1, 1);
	MainRoomData.RoomPosition = ChooseStartRoomPosition(MainRoomData.RoomSize);
	MainRoomData.TileType = RoomType ? RoomType->TileType : ETileType::Empty;
	MainRoomData.Directions = 0;

	if (!CanPlaceRoom(MainRoomData.RoomPosition, MainRoomData.RoomSize))
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Start room does not fit. RoomID=%s Size=(%d,%d)"),
			*MainRoomData.RoomID.ToString(), MainRoomData.RoomSize.X, MainRoomData.RoomSize.Y);
		return;
	}

	RoomQueue.Enqueue(MainRoomData);
	PlaceRoomData(MainRoomData);
	StartRoomData = MainRoomData;
	bHasStartRoomData = true;
	PlacedRoomCount = 1;

	UE_LOG(LogARPG_Map, Log, TEXT("Placed start room: %s at (%d, %d) size (%d, %d)"),
		*StartRoomData.RoomID.ToString(),
		StartRoomData.RoomPosition.X,
		StartRoomData.RoomPosition.Y,
		StartRoomData.RoomSize.X,
		StartRoomData.RoomSize.Y);

	while(RoomQueue.Dequeue(MainRoomData))
	{
		if (bEndPlaced)
		{
			break;
		}

		const FDiaAdjacencyRule& SelectRoom = FindAdjacencyRule(MainRoomData.RoomID);
		if (SelectRoom.SourceRoomID == NAME_None)
		{
			continue;
		}

		// Dir? North, East, South, West ?쒖꽌濡??쒗쉶??
		const UDiaRoomType* SourceRoomType = RoomDataCache.FindRef(MainRoomData.RoomID);
		const TArray<FDiaRoomPort> SourcePorts = GetRoomPorts(SourceRoomType);
		const int32 SourcePortStart = SourcePorts.IsEmpty() ? 0 : FMath::RandRange(0, SourcePorts.Num() - 1);

		for (int32 SourcePortIndex = 0; SourcePortIndex < SourcePorts.Num(); ++SourcePortIndex)
		{
			if (bEndPlaced)
			{
				break;
			}

			const FDiaRoomPort SourcePort = SourcePorts[(SourcePortStart + SourcePortIndex) % SourcePorts.Num()];
			if (MainRoomData.ActivePorts.Contains(SourcePort))
			{
				continue;
			}

			TArray<FDiaAdjacencyRule> Candidates;
			FindRoomCandidates(SelectRoom, Candidates, PlacedRoomCount);

			for (const FDiaAdjacencyRule& Candidate : Candidates)
			{
				if (PlacedRoomCount >= MaxGeneratedRooms && Candidate.SourceRoomID != TEXT("End"))
				{
					continue;
				}

				FDiaRoomData CandidateRoomData;
				FDiaRoomPort CandidatePort;
				if (!TryBuildConnectedRoomData(MainRoomData, SourcePort, Candidate, CandidateRoomData, CandidatePort))
				{
					continue;
				}

				if (!MainRoomData.ActivePorts.Contains(SourcePort))
				{
					MainRoomData.ActivePorts.Add(SourcePort);
				}
				MainRoomData.Directions = SourceRoomType
					? DiaMapGenerator::MakeDirectionByPorts(MainRoomData.ActivePorts, SourceRoomType->RoomSize, MainRoomData.RotateDegree)
					: MainRoomData.Directions;
				AddActivePortToPlacedRoom(MainRoomData.RoomPosition, SourcePort);

				PlaceRoomData(CandidateRoomData);
				++PlacedRoomCount;

				UE_LOG(LogARPG_Map, Log, TEXT("Placing Room: %s at (%d, %d) with size (%d, %d)"),
					*CandidateRoomData.RoomID.ToString(),
					CandidateRoomData.RoomPosition.X, CandidateRoomData.RoomPosition.Y,
					CandidateRoomData.RoomSize.X, CandidateRoomData.RoomSize.Y);

				if (CandidateRoomData.RoomID != TEXT("End"))
				{
					RoomQueue.Enqueue(CandidateRoomData);
				}
				else
				{
					bEndPlaced = true;
				}

				break;
			}
		}

		//留듭쓽 ?쇱젙 鍮꾩쑉 ?댁긽???앹꽦?섎㈃ 醫낅즺 (臾댄븳 猷⑦봽 諛⑹?)
		//end濡?怨좎젙?쒕떎
		if (bEndPlaced)
		{
			break;
		}
	}

	if (!bEndPlaced && !TryPlaceEndRoom(PlacedRoomCount))
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Generated map without an End room."));
	}

	ValidateAndRepairActivePortLinks();
	PrintMapAsText(); // ?앹꽦??留듭쓣 ?띿뒪?몃줈 異쒕젰
}

bool UDiaMapGeneratorSubsystem::TryPlaceEndRoom(int32 CurrentRooms)
{
	const FDiaAdjacencyRule EndRule = FindAdjacencyRule(TEXT("End"));
	if (EndRule.SourceRoomID == NAME_None)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: No adjacency rule found for End room."));
		return false;
	}

	const UDiaRoomType* EndRoomType = RoomDataCache.FindRef(EndRule.SourceRoomID);
	if (!EndRoomType)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: No room data found for End room."));
		return false;
	}

	{
		TArray<int32> PortSourceIndices;
		for (int32 Index = 0; Index < MapData.Num(); ++Index)
		{
			const FDiaRoomData& RoomData = MapData[Index];
			if (RoomData.RoomID != NAME_None && RoomData.RoomID != TEXT("End") && RoomData.bIsAnchor)
			{
				PortSourceIndices.Add(Index);
			}
		}

		const FIntPoint Center(MapWidth / 2, MapHeight / 2);
		PortSourceIndices.Sort([this, Center](const int32 A, const int32 B)
		{
			int32 AX = 0;
			int32 AY = 0;
			int32 BX = 0;
			int32 BY = 0;
			GetIndexToCoord(A, AX, AY);
			GetIndexToCoord(B, BX, BY);

			const int32 ADistance = FMath::Abs(AX - Center.X) + FMath::Abs(AY - Center.Y);
			const int32 BDistance = FMath::Abs(BX - Center.X) + FMath::Abs(BY - Center.Y);
			return ADistance > BDistance;
		});

		const auto SourceRuleAllowsEnd = [this](const FDiaRoomData& SourceRoom)
		{
			const FDiaAdjacencyRule SourceRule = FindAdjacencyRule(SourceRoom.RoomID);
			if (SourceRule.SourceRoomID == NAME_None)
			{
				return false;
			}

			for (const FRoomWeight& Entry : SourceRule.CandidateWeights)
			{
				if (Entry.RoomID == TEXT("End") && Entry.Weight > 0.f)
				{
					return true;
				}
			}

			return false;
		};

		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			const bool bRequireAuthoredEndCandidate = Pass == 0;
			for (const int32 SourceIndex : PortSourceIndices)
			{
				FDiaRoomData SourceRoom = MapData[SourceIndex];
				if (bRequireAuthoredEndCandidate && !SourceRuleAllowsEnd(SourceRoom))
				{
					continue;
				}

				const UDiaRoomType* SourceRoomType = RoomDataCache.FindRef(SourceRoom.RoomID);
				const TArray<FDiaRoomPort> SourcePorts = GetRoomPorts(SourceRoomType);
				for (const FDiaRoomPort& SourcePort : SourcePorts)
				{
					if (SourceRoom.ActivePorts.Contains(SourcePort))
					{
						continue;
					}

					FDiaRoomData EndRoomData;
					FDiaRoomPort EndPort;
					if (!TryBuildConnectedRoomData(SourceRoom, SourcePort, EndRule, EndRoomData, EndPort))
					{
						continue;
					}

					AddActivePortToPlacedRoom(SourceRoom.RoomPosition, SourcePort);
					PlaceRoomData(EndRoomData);

					UE_LOG(LogARPG_Map, Log, TEXT("End room fallback placement after %d rooms at (%d, %d)."), CurrentRooms, EndRoomData.RoomPosition.X, EndRoomData.RoomPosition.Y);
					return true;
				}
			}
		}
	}

	return false;
}

void UDiaMapGeneratorSubsystem::CreateMapFromData()
{
	using namespace DiaMapConstants;

	MapObjList.Empty();
	StartRoomActor = nullptr;

    for (const FDiaRoomData& SavedData : MapData)
    {
        if (SavedData.RoomID.IsNone() || !SavedData.bIsAnchor) continue;
        FDiaRoomData VisualData = SavedData;
        if (VisualData.TileType == ETileType::Corridor)
        {
            VisualData.Directions = MakeWorldDirectionsFromActivePorts(SavedData);
            CalcuateCorridorType(VisualData.Directions, VisualData.RotateDegree, VisualData.RoomID);
            const UDiaRoomType* Source = RoomDataCache.FindRef(SavedData.RoomID);
            const UDiaRoomType* Visual = RoomDataCache.FindRef(VisualData.RoomID);
            if (Source && Visual)
            {
                VisualData.ActivePorts.Reset();
                for (const FDiaRoomPort& Port : SavedData.ActivePorts)
                {
                    const FDiaRoomPort WorldPort = DiaMapGenerator::RotatePortDegree(Port, Source->RoomSize, SavedData.RotateDegree);
                    VisualData.ActivePorts.Add(DiaMapGenerator::RotatePortDegree(WorldPort, SavedData.RoomSize, -VisualData.RotateDegree));
                }
            }
        }
        if (UDiaRoomType* RoomType = RoomDataCache.FindRef(VisualData.RoomID))
        {
            if (ADiaRoomBase* Room = CreateRoomActor(RoomType, VisualData, DiaMapConstants::TileSize))
            {
                MapObjList.Add(SavedData.RoomGuid, Room);
                if (Room->IsStartRoom()) StartRoomActor = Room;
                if (const ADiaGameState* GS = GetWorld()->GetGameState<ADiaGameState>(); GS && GS->IsRoomCleared(SavedData.RoomGuid))
                    Room->OnBattleEnd(SavedData.RoomGuid);
            }
        }
    }
}

ADiaRoomBase* UDiaMapGeneratorSubsystem::CreateRoomActor(UDiaRoomType* RoomType, const FDiaRoomData& RoomData, float TileSize)
{
	UClass* RoomCls = RoomType->Roomclass.LoadSynchronous();
	if (!RoomCls)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Failed to load RoomClass for RoomID: %s"), *RoomType->RoomID.ToString());
		return nullptr;
	}

	const FVector SpawnLocation(
		(RoomData.RoomPosition.X + (RoomData.RoomSize.X - 1) * 0.5f) * TileSize,
		(RoomData.RoomPosition.Y + (RoomData.RoomSize.Y - 1) * 0.5f) * TileSize,
		0.f);
	const FRotator SpawnRotation(0.f, RoomData.RotateDegree, 0.f);
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	ADiaRoomBase* RoomActor = GetWorld()->SpawnActorDeferred<ADiaRoomBase>(RoomCls, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!RoomActor)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Failed to spawn Room Actor for RoomID: %s"), *RoomType->RoomID.ToString());
		return nullptr;
	}
	 
	RoomActor->SetRoomSize(GetSafeRoomSize(RoomType->RoomSize));
	RoomActor->SetDoorPorts(GetRoomPorts(RoomType), RoomData.ActivePorts);
	RoomActor->SetTileType(RoomData.TileType);
    RoomActor->SetRoomGuid(RoomData.RoomGuid);
    RoomActor->SetSpawnType(RoomType->SpawnType);
    const bool bStart = bHasStartRoomData && RoomData.RoomPosition == StartRoomData.RoomPosition;
    RoomActor->SetRoomRole(bStart ? EDiaRoomRole::Start : RoomData.RoomID == TEXT("End") ? EDiaRoomRole::StageClear
        : RoomData.bTreasureRoom ? EDiaRoomRole::Treasure : EDiaRoomRole::Normal);
	RoomActor->FinishSpawning(SpawnTransform);

	return RoomActor;
}

bool UDiaMapGeneratorSubsystem::TryGetStartRoomWorldLocation(FVector& OutLocation, float ZOffset) const
{
	if (!bHasStartRoomData)
	{
		return false;
	}

	using namespace DiaMapConstants;

	const FIntPoint SafeSize = GetSafeRoomSize(StartRoomData.RoomSize);
	OutLocation = FVector(
		(StartRoomData.RoomPosition.X + (SafeSize.X - 1) * 0.5f) * TileSize,
		(StartRoomData.RoomPosition.Y + (SafeSize.Y - 1) * 0.5f) * TileSize,
		ZOffset);
	return true;
}

void UDiaMapGeneratorSubsystem::MovePlayerToStartRoom()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector StartLocation;
	if (!TryGetStartRoomWorldLocation(StartLocation))
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Cannot move player because start room data is missing."));
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogARPG_Map, Verbose, TEXT("UDiaMapGeneratorSubsystem: Cannot move player because player controller is missing."));
		return;
	}

	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogARPG_Map, Verbose, TEXT("UDiaMapGeneratorSubsystem: Cannot move player because pawn is missing."));
		return;
	}

	if (const UCapsuleComponent* CapsuleComponent = Pawn->FindComponentByClass<UCapsuleComponent>())
	{
		StartLocation.Z = FMath::Max(StartLocation.Z, CapsuleComponent->GetScaledCapsuleHalfHeight() + 5.f);
	}

	Pawn->SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
	UE_LOG(LogARPG_Map, Log, TEXT("Moved player to start room at %s"), *StartLocation.ToString());
}

void UDiaMapGeneratorSubsystem::CalcuateCorridorType(uint8 Directions, int32& OutRotateDegree, FName& OutRoomID) const
{
	const TArray<EDiaDirection>& EDirections = DiaMapGenerator::GetDirections(Directions);
	const int32 Count = EDirections.Num();
	UE_LOG(LogARPG_Map, Log, TEXT("Calculating Corridor Type for Directions: %d (Count: %d)"), Directions, Count);
	switch (Count)
	{
	case 1:
		OutRoomID = TEXT("None");
		break;
	case 2:
	{
		// North=+X, East=+Y 醫뚰몴怨?湲곗? DefaultShapeBit
		// 吏곸꽑: 硫붿떆媛 짹X濡??대┝ ??N|S = 0b0101
		// ?댁옄: 硫붿떆媛 ??遺곸쑝濡??대┝ ??W|N = 0b1001. 硫붿떆 湲곗? Yaw +90째 蹂댁젙.
		const bool bStraight = (GetOppositeDirection(EDirections[0]) == EDirections[1]);
		const uint8 DefaultShapeBit = bStraight ? static_cast<uint8>(0b0101) : static_cast<uint8>(0b1001);
		OutRoomID = bStraight ? TEXT("StraightCorridor") : TEXT("CornerCorridor");
		CalcuateCorridorDegree(DefaultShapeBit, Directions, OutRotateDegree);
		if (!bStraight)
		{
			OutRotateDegree = (OutRotateDegree + 90) % 360;
		}
	}
	break;
	case 3:
	{
		// T?? 硫붿떆媛 -X, +Y, -Y濡??대┝ ??S|E|W = 0b1110. 硫붿떆 湲곗? Yaw -90째 蹂댁젙.
		const uint8 DefaultShapeBit = 0b1110;
		OutRoomID = TEXT("TCorridor");
		CalcuateCorridorDegree(DefaultShapeBit, Directions, OutRotateDegree);
		OutRotateDegree = (OutRotateDegree - 90 + 360) % 360;
	}
	break;
	case 4:
		//Cross紐⑥뼇 return;
		OutRotateDegree = 0;
		OutRoomID = TEXT("CrossCorridor");
		break;
	}
}

void UDiaMapGeneratorSubsystem::CalcuateCorridorDegree(uint8 SourceDirections, uint8 DestDirections, int32& OutRotateDegree) const
{
	for(int32 i = 0; i < 360; i += 90)
	{
		uint8 RotatedSource = DiaMapGenerator::RotateDirectionsDegree(i, SourceDirections);
		if(RotatedSource == DestDirections)
		{
			OutRotateDegree = i;
			return;
		}
	}
}

TObjectPtr<ADiaRoomBase> UDiaMapGeneratorSubsystem::GetRoomActor(const FGuid& RoomGuid) const
{
	if (const TObjectPtr<ADiaRoomBase>* FoundActor = MapObjList.Find(RoomGuid))
	{
		return *FoundActor;
	}
	return nullptr;
}

FDiaAdjacencyRule UDiaMapGeneratorSubsystem::FindAdjacencyRule(const FName& RoomID) const
{
	for (const FDiaAdjacencyRule& Rule : AdjacencyRules)
	{
		if (Rule.SourceRoomID == RoomID)
		{
			return Rule;
		}
	}
	return FDiaAdjacencyRule();
}

void UDiaMapGeneratorSubsystem::FindRoomCandidates(const FDiaAdjacencyRule& Rule, TArray<FDiaAdjacencyRule>& OutCandidates, int32 CurrentRooms) const
{
	OutCandidates.Reset();
	TArray<FRoomWeight> WeightedEntries;
	const int32 MaxRooms = MapWidth * MapHeight;
	float TotalWeight = 0.f;

	for (const FRoomWeight& Entry : Rule.CandidateWeights)
	{
		if (Entry.Weight <= 0.f)
		{
			continue;
		}

		if (CurrentRooms >= MaxRooms && Entry.RoomID != TEXT("End"))
		{
			continue;
		}

		if ((CurrentRooms < MinMapRoomCount || CountRewardEligibleRooms() < TargetCombatRoomCount + 1) && Entry.RoomID == TEXT("End"))
		{
			continue;
		}

		const FDiaAdjacencyRule CandidateRule = FindAdjacencyRule(Entry.RoomID);
		if (CandidateRule.SourceRoomID == NAME_None)
		{
			continue;
		}

        const UDiaRoomType* CandidateType = RoomDataCache.FindRef(Entry.RoomID);
        if (CandidateType && CandidateType->TileType == ETileType::Floor && Entry.RoomID != TEXT("End")
            && CountRewardEligibleRooms() >= TargetCombatRoomCount + 1) continue;
		WeightedEntries.Add(Entry);
		TotalWeight += Entry.Weight;
	}

	while (WeightedEntries.Num() > 0 && TotalWeight > 0.f)
	{
		const float RandomValue = FMath::FRandRange(0.f, TotalWeight);
		float Acc = 0.f;
		int32 ChosenIndex = INDEX_NONE;

		for (int32 Index = 0; Index < WeightedEntries.Num(); ++Index)
		{
			Acc += WeightedEntries[Index].Weight;
			if (RandomValue <= Acc)
			{
				ChosenIndex = Index;
				break;
			}
		}

		if (ChosenIndex == INDEX_NONE)
		{
			ChosenIndex = WeightedEntries.Num() - 1;
		}

		const FRoomWeight ChosenEntry = WeightedEntries[ChosenIndex];
		TotalWeight -= ChosenEntry.Weight;
		WeightedEntries.RemoveAt(ChosenIndex);

		const FDiaAdjacencyRule ChosenRule = FindAdjacencyRule(ChosenEntry.RoomID);
		if (ChosenRule.SourceRoomID != NAME_None)
		{
			OutCandidates.Add(ChosenRule);
		}
	}
}

void UDiaMapGeneratorSubsystem::LoadRoomData(const FName& RoomID)
{
	FPrimaryAssetId AssetId = FPrimaryAssetId("DiaMap", RoomID);
	UAssetManager& Manager = UAssetManager::Get();

	TSharedPtr<FStreamableHandle> Handle = Manager.LoadPrimaryAsset(AssetId);
	if (Handle.IsValid())
	{
		Handle->WaitUntilComplete();
	}

	UDiaRoomType* LoadedTheme = Cast<UDiaRoomType>(Manager.GetPrimaryAssetObject(AssetId));
	if (!LoadedTheme)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("LoadRoomData: Failed to load DiaRoomType. AssetId=%s"), *AssetId.ToString());
		return;
	}

	RoomDataCache.Add(RoomID, LoadedTheme);
}

void UDiaMapGeneratorSubsystem::PrintMapAsText() const
{
	FString Output;
	Output += TEXT("=== Generated Map ===\n");
	
	for (int32 Y = 0; Y < MapHeight; ++Y)
	{
		FString Row;
		for (int32 X = 0; X < MapWidth; ++X)
		{
			int32 Index = GetIndex(X, Y);
			if (Index < MapData.Num() && MapData[Index].RoomID != NAME_None)
			{
				FString RoomName = MapData[Index].RoomID.ToString();
				Row += FString::Printf(TEXT("[%s] "), *RoomName.Left(4)); // 泥섏쓬 4湲?먮쭔
			}
			else
			{
				Row += TEXT("[----] ");
			}
		}
		Output += Row + TEXT("\n");
	}
	
	UE_LOG(LogARPG_Map, Log, TEXT("\n%s"), *Output);
}

void UDiaMapGeneratorSubsystem::SaveMapToFile(const FString& FilePath) const
{
	FString Output;
	Output += TEXT("=== Generated Map ===\n");
	Output += FString::Printf(TEXT("Size: %dx%d\n"), MapWidth, MapHeight);
	Output += FString::Printf(TEXT("MinRooms: %d\n"), MinMapRoomCount);
	Output += FString::Printf(TEXT("TargetFillRatio: %.2f\n\n"), TargetFillRatio);
	if (bHasStartRoomData)
	{
		Output += FString::Printf(TEXT("StartRoom: %s at (%d,%d) [%dx%d]\n\n"),
			*StartRoomData.RoomID.ToString(),
			StartRoomData.RoomPosition.X,
			StartRoomData.RoomPosition.Y,
			StartRoomData.RoomSize.X,
			StartRoomData.RoomSize.Y);
	}
	
	for (int32 Y = 0; Y < MapHeight; ++Y)
	{
		for (int32 X = 0; X < MapWidth; ++X)
		{
			int32 Index = GetIndex(X, Y);
			if (Index < MapData.Num() && MapData[Index].RoomID != NAME_None)
			{
				Output += FString::Printf(TEXT("(%d,%d): %s [%dx%d]\n"), 
					X, Y, 
					*MapData[Index].RoomID.ToString(),
					MapData[Index].RoomSize.X, 
					MapData[Index].RoomSize.Y);
			}
		}
	}
	
	FFileHelper::SaveStringToFile(Output, *FilePath);
	UE_LOG(LogARPG_Map, Log, TEXT("Map saved to: %s"), *FilePath);
}



int32 UDiaMapGeneratorSubsystem::CountRewardEligibleRooms() const
{
    int32 Count = 0;
    for (const FDiaRoomData& Room : MapData)
        if (Room.bIsAnchor && !Room.RoomID.IsNone() && Room.TileType == ETileType::Floor
            && Room.RoomID != TEXT("End") && Room.RoomPosition != StartRoomData.RoomPosition) ++Count;
    return Count;
}

void UDiaMapGeneratorSubsystem::GeneratePlayableFloor(FName MapID)
{
    TArray<FDiaRoomData> BestLayout;
    FDiaRoomData BestStart;
    int32 BestScore = -1;
    for (int32 Attempt = 0; Attempt < 32; ++Attempt)
    {
        BFSGenerateMap(MapID);
        const bool bHasBoss = MapData.ContainsByPredicate([](const FDiaRoomData& R) { return R.bIsAnchor && R.RoomID == TEXT("End"); });
        const int32 Score = CountRewardEligibleRooms() + (bHasBoss ? 1000 : 0);
        if (bHasStartRoomData && Score > BestScore)
        {
            BestScore = Score;
            BestLayout = MapData;
            BestStart = StartRoomData;
        }
        if (bHasBoss && CountRewardEligibleRooms() >= TargetCombatRoomCount + 1) break;
    }
    if (!BestLayout.IsEmpty())
    {
        MapData = MoveTemp(BestLayout);
        StartRoomData = BestStart;
        bHasStartRoomData = true;
    }
    AssignFloorRoles();
    if (BestScore < 1000 + TargetCombatRoomCount + 1)
        UE_LOG(LogARPG_Map, Warning, TEXT("Floor room target could not fit authored rooms in %dx%d. Eligible=%d, boss=%d. Increase grid or adjust room ports."),
            MapWidth, MapHeight, CountRewardEligibleRooms(), BestScore >= 1000);
}

void UDiaMapGeneratorSubsystem::AssignFloorRoles()
{
    if (!bHasStartRoomData) return;
    const int32 Start = GetIndex(StartRoomData.RoomPosition.X, StartRoomData.RoomPosition.Y);
    TMap<int32, int32> Parents;
    TArray<int32> Queue;
    Parents.Add(Start, INDEX_NONE);
    Queue.Add(Start);
    int32 Boss = INDEX_NONE;
    for (int32 Head = 0; Head < Queue.Num(); ++Head)
    {
        const int32 Index = Queue[Head];
        const FDiaRoomData& Room = MapData[Index];
        if (Room.RoomID == TEXT("End")) Boss = Index;
        const UDiaRoomType* Type = RoomDataCache.FindRef(Room.RoomID);
        if (!Type) continue;
        for (const FDiaRoomPort& Port : Room.ActivePorts)
        {
            const FDiaRoomPort WorldPort = DiaMapGenerator::RotatePortDegree(Port, Type->RoomSize, Room.RotateDegree);
            const FIntPoint Cell = Room.RoomPosition + WorldPort.LocalCell + DiaMapGenerator::GetDirectionOffset(WorldPort.Direction);
            if (Cell.X < 0 || Cell.Y < 0 || Cell.X >= MapWidth || Cell.Y >= MapHeight) continue;
            const FDiaRoomData& Neighbor = MapData[GetIndex(Cell.X, Cell.Y)];
            if (Neighbor.RoomID.IsNone()) continue;
            const int32 Next = GetIndex(Neighbor.RoomPosition.X, Neighbor.RoomPosition.Y);
            if (!Parents.Contains(Next)) { Parents.Add(Next, Index); Queue.Add(Next); }
        }
    }
    TSet<int32> BossPath;
    for (int32 Index = Boss; Index != INDEX_NONE; Index = Parents.FindChecked(Index)) BossPath.Add(Index);
    int32 Treasure = INDEX_NONE;
    int32 BestScore = -1;
    for (const int32 Index : Queue)
    {
        const FDiaRoomData& Room = MapData[Index];
        if (Index == Start || Index == Boss || Room.TileType != ETileType::Floor) continue;
        const int32 Score = (!BossPath.Contains(Index) ? 1000 : 0) + (Room.ActivePorts.Num() == 1 ? 100 : 0)
            + FMath::Abs(Room.RoomPosition.X - StartRoomData.RoomPosition.X) + FMath::Abs(Room.RoomPosition.Y - StartRoomData.RoomPosition.Y);
        if (Score > BestScore) { BestScore = Score; Treasure = Index; }
    }
    for (int32 Index = 0; Index < MapData.Num(); ++Index)
    {
        FDiaRoomData& Room = MapData[Index];
        if (!Room.bIsAnchor || Room.RoomID.IsNone()) continue;
        Room.RoomGuid = FGuid::NewGuid();
        Room.bTreasureRoom = Index == Treasure;
        const FDiaRoomData Copy = Room;
        UpdatePlacedRoomData(Copy);
    }
    StartRoomData = MapData[Start];
    if (Treasure == INDEX_NONE) UE_LOG(LogARPG_Map, Error, TEXT("No reachable floor room available for treasure."));
}
