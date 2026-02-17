// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DiaMapGeneratorSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Types/MapGenerate.h"
#include "DataAsset/DiaRoomType.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

DEFINE_LOG_CATEGORY_STATIC(LogARPG_Map, Log, All);

void UDiaMapGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MapData.SetNum(MapWidth * MapHeight);

	LoadAdjacencyRules();
}

void UDiaMapGeneratorSubsystem::Deinitialize()
{
}

void UDiaMapGeneratorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	BFSGenerateMap(FName("Entrance"));
	SaveMapToFile(FPaths::ProjectSavedDir() / TEXT("LastGeneratedMap.txt"));
	CreateMapFromData();
}

void UDiaMapGeneratorSubsystem::LoadAdjacencyRules()
{
	if (!MapAdjacencyTable.IsValid())
	{
		// 기본 DataTable 경로 설정
		MapAdjacencyTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Datatable/DT_DiaAdjacencyRules.DT_DiaAdjacencyRules")));
	}

	UDataTable* DataTable = MapAdjacencyTable.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Failed to load DataTable at path: %s"),
			*MapAdjacencyTable.ToString());
		return;
	}

	// 모든 행 가져오기
	TArray<FName> RowNames = DataTable->GetRowNames();
	AdjacencyRules.Empty();

	for (const FName& RowName : RowNames)
	{
		FDiaAdjacencyRule* RowData = DataTable->FindRow<FDiaAdjacencyRule>(RowName, TEXT("UDiaMapGeneratorSubsystem"));
		if (RowData)
		{
			AdjacencyRules.Add(*RowData);
			LoadRoomData(RowData->SourceRoomID);
		}
		
	}
}

//Soruce RoomID로 Map을 Generate하자.
void UDiaMapGeneratorSubsystem::GenerateMap(FName MapID)
{
	const int32 MapSize = MapWidth * MapHeight;
	MapData.Empty();
	MapData.SetNum(MapSize);

	const FDiaAdjacencyRule& MainRoom = FindAdjacencyRule(MapID);
	if(MainRoom.SourceRoomID == NAME_None)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: No adjacency rule found for RoomID: %s"), *MapID.ToString());
		return;
	}

	int32 StartIndex = MapSize / 2; // 중앙에서 시작

	UDiaRoomType* RoomType = RoomDataCache.FindRef(MainRoom.SourceRoomID);
	MapData[StartIndex].RoomID = MainRoom.SourceRoomID;
	MapData[StartIndex].RoomSize = RoomType ? RoomType->RoomSize : FIntPoint(1, 1);

	FDiaAdjacencyRule Room = MainRoom;

	while (true)
	{
		TArray<FDiaAdjacencyRule> Candidates;
		FindRoomCandidates(Room, Candidates);
		if (Candidates.Num() == 0)
		{
			break;
		}

		//이거 이렇게 놓으면 문제가, 먼저  탐색되는 방향이 선호되는 현상이 나타날수밖에 없음.
		//수정 필수 HACK.
		EDiaDirection dir = CanConnectRooms(Room, Candidates[0]);
		if (dir == EDiaDirection::None)
		{
			break;
		}

		int32 DirectionX = dir == EDiaDirection::East ? 1 : (dir == EDiaDirection::West ? -1 : 0);
		int32 DirectionY = dir == EDiaDirection::North ? -1 : (dir == EDiaDirection::South ? 1 : 0);

		int32 NextX = StartIndex % MapWidth + DirectionX;
		int32 NextY = StartIndex / MapWidth + DirectionY;

		if (CanPlaceRoom(Candidates[0], NextX, NextY))
		{
			int32 NextIndex = GetIndex(NextX, NextY);
			MapData[NextIndex].RoomID = Candidates[0].SourceRoomID;

			UDiaRoomType* NextRoomType = RoomDataCache.FindRef(Candidates[0].SourceRoomID);
			MapData[NextIndex].RoomSize = NextRoomType ? NextRoomType->RoomSize : FIntPoint(1, 1);

			StartIndex = NextIndex; // 다음 위치로 이동
			Room = Candidates[0]; // 다음 룸으로 업데이트
		}

		if (Candidates[0].SourceRoomID == TEXT("End"))
		{
			break;
		}
	}
	
	PrintMapAsText(); // 생성된 맵을 텍스트로 출력
}

//단순히 빈 공간인지 체크.
bool UDiaMapGeneratorSubsystem::CanPlaceRoom(const FDiaAdjacencyRule& Rule, int32 NextX, int32 NextY) const
{
	if(NextX < 0 || NextX >= MapWidth || NextY < 0 || NextY >= MapHeight)
	{
		return false;
	}
	if(MapData[GetIndex(NextX, NextY)].RoomID != NAME_None)
	{
		return false;
	}

	return true;
}

//현재 room의 가능한 방향, 다음 위치가 룸이 놓일 수 있는지.
EDiaDirection UDiaMapGeneratorSubsystem::CanConnectRooms(const FDiaAdjacencyRule& SourceRule, const FDiaAdjacencyRule& DestRule) const
{
	//int32 DirectionX = Dir == EDiaDirection::East ? 1 : (Dir == EDiaDirection::West ? -1 : 0);
	//int32 DirectionY = Dir == EDiaDirection::North ? -1 : (Dir == EDiaDirection::South ? 1 : 0);

	for(EDiaDirection SourceDir : SourceRule.Directions)
	{

		for(EDiaDirection DestDir : DestRule.Directions)
		{
			if(GetOppositeDirection(SourceDir) == DestDir)
			{
				return SourceDir;
			}
		}
	} 

	return EDiaDirection::None;
}

void UDiaMapGeneratorSubsystem::BFSGenerateMap(FName MapID)
{
	int32 CreatedRooms = 0;
	const int32 MapSize = MapWidth * MapHeight;
	MapData.Empty();
	MapData.SetNum(MapSize);

	TQueue<FDiaRoomData> RoomQueue;
	const FDiaAdjacencyRule& MainRoom = FindAdjacencyRule(MapID);
	UDiaRoomType* RoomType = RoomDataCache.FindRef(MainRoom.SourceRoomID);

	FDiaRoomData MainRoomData;
	MainRoomData.RoomID = MainRoom.SourceRoomID;
	MainRoomData.RoomSize = RoomType ? RoomType->RoomSize : FIntPoint(1, 1);
	MainRoomData.RoomPosition = FIntPoint(MapWidth / 2, MapHeight / 2); // 중앙에 배치

	RoomQueue.Enqueue(MainRoomData);
	MapData[GetIndex(MainRoomData.RoomPosition.X, MainRoomData.RoomPosition.Y)] = MainRoomData;

	while(RoomQueue.Dequeue(MainRoomData))
	{
		TArray<FDiaAdjacencyRule> Candidates;
		const FDiaAdjacencyRule& SelectRoom = FindAdjacencyRule(MainRoomData.RoomID);
		FindRoomCandidates(SelectRoom, Candidates);
		int32 SelectIndex = GetIndex(MainRoomData.RoomPosition.X, MainRoomData.RoomPosition.Y);

		for(const FDiaAdjacencyRule& Candidate : Candidates)
		{
			EDiaDirection dir = CanConnectRooms(SelectRoom, Candidate);
			if(dir != EDiaDirection::None)
			{
				int32 DirectionX = dir == EDiaDirection::East ? 1 : (dir == EDiaDirection::West ? -1 : 0);
				int32 DirectionY = dir == EDiaDirection::North ? -1 : (dir == EDiaDirection::South ? 1 : 0);

				int32 NextX = SelectIndex % MapWidth + DirectionX;
				int32 NextY = SelectIndex / MapWidth + DirectionY;

				if (CanPlaceRoom(Candidate, NextX, NextY))
				{
					UDiaRoomType* NextRoomType = RoomDataCache.FindRef(Candidate.SourceRoomID);

					FDiaRoomData NextRoomData;
					NextRoomData.RoomID = Candidate.SourceRoomID;
					NextRoomData.RoomSize = NextRoomType ? NextRoomType->RoomSize : FIntPoint(1, 1);
					NextRoomData.RoomPosition = FIntPoint(NextX, NextY);
					
					//size 에러가 나기 때문에 로깅

					UE_LOG(LogARPG_Map, Log, TEXT("Placing Room: %s at (%d, %d) with size (%d, %d)"),
						*NextRoomData.RoomID.ToString(),
						NextX, NextY,
						NextRoomData.RoomSize.X, NextRoomData.RoomSize.Y);

					MapData[GetIndex(NextX, NextY)] = NextRoomData;

					if (NextRoomData.RoomID != TEXT("End"))
					{
						RoomQueue.Enqueue(NextRoomData);
						++CreatedRooms;
					}
					else
					{
						return;
					}
				}
			}
		}

		//맵의 일정 비율 이상이 생성되면 종료 (무한 루프 방지)
		if (CreatedRooms >= MapSize * 0.6)
		{
			break;
		}
	}

	PrintMapAsText(); // 생성된 맵을 텍스트로 출력
}

void UDiaMapGeneratorSubsystem::CreateMapFromData()
{
	const float TileSize = 1000.f;
	for(const FDiaRoomData& RoomData : MapData)
	{
		if(RoomData.RoomID != NAME_None)
		{
			UDiaRoomType* RoomType = RoomDataCache.FindRef(RoomData.RoomID);
			UE_LOG(LogARPG_Map, Log, TEXT("RoomType for %s is %s"), *RoomData.RoomID.ToString(), RoomType ? *RoomType->GetName() : TEXT("nullptr"));
			if (RoomType)
			{
				CraeteRoomActor(RoomType, RoomData.RoomPosition, TileSize);
			}
		}
	}
}

void UDiaMapGeneratorSubsystem::CraeteRoomActor(UDiaRoomType* RoomType, const FIntPoint& RoomPosition, float TileSize)
{
	UClass* RoomCls = RoomType->Roomclass.LoadSynchronous();
	UE_LOG(LogARPG_Map, Log, TEXT("Loaded Room Class for %s: %s"), *RoomType->GetName(), RoomCls ? *RoomCls->GetName() : TEXT("nullptr"));
	if (RoomCls)
	{
		FVector SpawnLocation = FVector(RoomPosition.X * TileSize, RoomPosition.Y * TileSize, 0.f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		GetWorld()->SpawnActor<AActor>(RoomCls, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		UE_LOG(LogARPG_Map, Log, TEXT("Spawned Room Actor: %s at location (%f, %f)"), *RoomCls->GetName(), SpawnLocation.X, SpawnLocation.Y);
	}
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

void UDiaMapGeneratorSubsystem::FindRoomCandidates(const FDiaAdjacencyRule& Rule, TArray<FDiaAdjacencyRule>& OutCandidates) const
{
	OutCandidates.Reset();

	float TotalWeight = 0.f;
	for (const auto& Pair : Rule.CandidateWeights)
	{
		TotalWeight += Pair.Value;
	}
	if (TotalWeight <= 0.f) return;

	float RandomValue = FMath::FRandRange(0.f, TotalWeight);
	float Acc = 0.f;
	FName ChosenID = NAME_None;
	for (const auto& Pair : Rule.CandidateWeights)
	{
		Acc += Pair.Value;
		if (RandomValue <= Acc)
		{
			ChosenID = Pair.Key;
			break;
		}
	}

	if (ChosenID != NAME_None)
	{
		FDiaAdjacencyRule ChosenRule = FindAdjacencyRule(ChosenID);
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
				Row += FString::Printf(TEXT("[%s] "), *RoomName.Left(4)); // 처음 4글자만
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
	Output += FString::Printf(TEXT("Size: %dx%d\n\n"), MapWidth, MapHeight);
	
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

