// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DiaMapGeneratorSubsystem.h"
#include "Engine/DataTable.h"
#include "Engine/AssetManager.h"
#include "Types/MapGenerate.h"
#include "DataAsset/DiaRoomType.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

#include "Map/DiaRoomBase.h"

#include "System/MonsterSpawnSubSystem.h"
#include "System/MapInfoSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogARPG_Map, Log, All);

void UDiaMapGeneratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//여기서 MonsterSpawnSubSystem을 먼저 초기화한다.
	//여기서 각 Room에 Spawn될 몬스터 그룹 정보를 로드할 수 있도록 하기 위해서.
	Collection.InitializeDependency<UMonsterSpawnSubSystem>();

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
	int32 CurrentRooms = 0;
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
		FindRoomCandidates(Room, Candidates, CurrentRooms);
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

		int32 DirectionX = dir == EDiaDirection::North ? 1 : (dir == EDiaDirection::South ? -1 : 0);
		int32 DirectionY = dir == EDiaDirection::East ? 1 : (dir == EDiaDirection::West ? -1 : 0);

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
		CurrentRooms++;
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

//이건 그냥 bool값 넘겨도 되는데 일단 일관성 유지, 근데 몇 도 만큼 회전했는지는 반환 해야함.
bool UDiaMapGeneratorSubsystem::CanConnectRooms(const FDiaRoomData& SourceRoom, const FDiaRoomData& DestRoom, const EDiaDirection Direction, int32& OutRotateDegree) const
{
	for (int32 i = 0; i < 360; i += 90)
	{
		uint8 DestDirection = DiaMapGenerator::RotateDirectionsDegree(i, DestRoom.Directions);
		TArray<EDiaDirection> DestDirections = DiaMapGenerator::GetDirections(DestDirection);

		for (const EDiaDirection& DestDir : DestDirections)
		{
			if (GetOppositeDirection(Direction) == DestDir)
			{
				OutRotateDegree = i;
				return true;
			}
		}
	}

	OutRotateDegree = 0;
	return false;
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
	MainRoomData.Directions = DiaMapGenerator::AllDirectionsBit;

	RoomQueue.Enqueue(MainRoomData);
	MapData[GetIndex(MainRoomData.RoomPosition.X, MainRoomData.RoomPosition.Y)] = MainRoomData;

	while(RoomQueue.Dequeue(MainRoomData))
	{
		TArray<FDiaAdjacencyRule> Candidates;
		const FDiaAdjacencyRule& SelectRoom = FindAdjacencyRule(MainRoomData.RoomID);
		FindRoomCandidates(SelectRoom, Candidates, CreatedRooms);
		int32 SelectIndex = GetIndex(MainRoomData.RoomPosition.X, MainRoomData.RoomPosition.Y);

		// Dir은 North, East, South, West 순서로 순회됨
		for (const EDiaDirection& Dir : GetAllDirections())
		{
			//현재 MainRoom이 해당 방향으로 연결 가능한지 체크.
			uint8 Direction = DiaMapGenerator::SetDirection(Dir);
			if(MainRoomData.Directions & Direction)
			{
				for (const FDiaAdjacencyRule& Candidate : Candidates)
				{
					const UDiaRoomType* CandidateRoomType = RoomDataCache.FindRef(Candidate.SourceRoomID);
					if (!CandidateRoomType)
					{
						continue;
					}

					FDiaRoomData CandidateRoomData;
					CandidateRoomData.RoomID = Candidate.SourceRoomID;
					CandidateRoomData.RoomSize = CandidateRoomType->RoomSize;
					CandidateRoomData.RoomPosition = MainRoomData.RoomPosition; // 임시
					CandidateRoomData.Directions = DiaMapGenerator::MakeDirectionByArray(Candidate.Directions);
					CandidateRoomData.TileType = CandidateRoomType->TileType;

					int32 RotateDegree = 0;
					if (CanConnectRooms(MainRoomData, CandidateRoomData, Dir, RotateDegree))
					{
						int32 DirectionX = Dir == EDiaDirection::North ? 1 : (Dir == EDiaDirection::South ? -1 : 0);
						int32 DirectionY = Dir == EDiaDirection::East ? 1 : (Dir == EDiaDirection::West ? -1 : 0);

						int32 NextX = SelectIndex % MapWidth + DirectionX;
						int32 NextY = SelectIndex / MapWidth + DirectionY;

						//돌아간 각도 및, 돌아간 방향으로 Direction 체크하고 업데이트
						if (CanPlaceRoom(Candidate, NextX, NextY))
						{
							CandidateRoomData.RoomPosition = FIntPoint(NextX, NextY);
							CandidateRoomData.RotateDegree = RotateDegree;
							CandidateRoomData.Directions = DiaMapGenerator::RotateDirectionsDegree(RotateDegree, CandidateRoomData.Directions);

							MapData[GetIndex(NextX, NextY)] = CandidateRoomData;

							UE_LOG(LogARPG_Map, Log, TEXT("Placing Room: %s at (%d, %d) with size (%d, %d)"),
								*CandidateRoomData.RoomID.ToString(),
								NextX, NextY,
								CandidateRoomData.RoomSize.X, CandidateRoomData.RoomSize.Y);

							if (CandidateRoomData.RoomID != TEXT("End"))
							{
								RoomQueue.Enqueue(CandidateRoomData);
								++CreatedRooms;
							}
							else
							{
								return;
							}

						}
					}
				}
			}
		}

		//맵의 일정 비율 이상이 생성되면 종료 (무한 루프 방지)
		//end로 고정한다
		if (CreatedRooms >= MapSize * 0.6)
		{
			MainRoomData.RoomID = TEXT("End");
			break;
		}
	}

	PrintMapAsText(); // 생성된 맵을 텍스트로 출력
}

void UDiaMapGeneratorSubsystem::CreateMapFromData()
{
	using namespace DiaMapConstants;

	const UMonsterSpawnSubSystem* MonsterSpawnSubSystem = GetWorld()->GetSubsystem<UMonsterSpawnSubSystem>();
	const UMapInfoSubsystem* MapInfoSubsystem = GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UMapInfoSubsystem>() : nullptr;
	if (!MonsterSpawnSubSystem || !MapInfoSubsystem)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Failed to get UMonsterSpawnSubSystem"));
		return;
	}

	FName MapID = MapInfoSubsystem->GetCurrentMapID();

	for(FDiaRoomData& RoomData : MapData)
	{
		if(RoomData.RoomID != NAME_None)
		{
			//복도의 경우에는 맵이 완성된 후에 더 체크한다.
			if(RoomData.TileType == ETileType::Corridor)
			{
				RoomData.RoomID = TEXT("None");
				uint8 Directions = 0;
				CheckConnectedPointCount(RoomData.RoomPosition.X, RoomData.RoomPosition.Y, Directions, RoomData);
				RoomData.Directions = Directions;
			}

			UDiaRoomType* RoomType = RoomDataCache.FindRef(RoomData.RoomID);
			if (RoomType)
			{
				FName SpawnGroupName = NAME_None;
				const TArray<FMapSpawnInfo>& SpawnInfo = MonsterSpawnSubSystem->GetSpawnInfosForMap(MapID);
				if (SpawnInfo.Num() > 0)
				{
					const int32 RandSpawnInfoIndex = FMath::RandRange(0, SpawnInfo.Num() - 1);
					SpawnGroupName = SpawnInfo[RandSpawnInfoIndex].GroupName;
				}

				ADiaRoomBase* RoomBase = CraeteRoomActor(RoomType, RoomData, DiaMapConstants::TileSize, SpawnGroupName);
				if (RoomBase)
				{
					FGuid RoomGuid = FGuid::NewGuid();
					RoomBase->SetRoomGuid(RoomGuid);
					MapObjList.Add(RoomGuid, RoomBase);
				}
			}
		}
	}
}

ADiaRoomBase* UDiaMapGeneratorSubsystem::CraeteRoomActor(UDiaRoomType* RoomType, const FDiaRoomData& RoomData, float TileSize, FName SpawnGroupName)
{
	UClass* RoomCls = RoomType->Roomclass.LoadSynchronous();
	if (!RoomCls)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Failed to load RoomClass for RoomID: %s"), *RoomType->RoomID.ToString());
		return nullptr;
	}

	const FVector SpawnLocation(RoomData.RoomPosition.X * TileSize, RoomData.RoomPosition.Y * TileSize, 0.f);
	const FRotator SpawnRotation(0.f, RoomData.RotateDegree, 0.f);
	const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

	ADiaRoomBase* RoomActor = GetWorld()->SpawnActorDeferred<ADiaRoomBase>(RoomCls, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!RoomActor)
	{
		UE_LOG(LogARPG_Map, Warning, TEXT("UDiaMapGeneratorSubsystem: Failed to spawn Room Actor for RoomID: %s"), *RoomType->RoomID.ToString());
		return nullptr;
	}
	 
	if (RoomData.TileType == ETileType::Floor)
	{
		RoomActor->SetDoorDirections(RoomData.Directions);
	}

	RoomActor->SetTileType(RoomData.TileType);

	RoomActor->FinishSpawning(SpawnTransform);

	return RoomActor;
}

void UDiaMapGeneratorSubsystem::CheckConnectedPointCount(int32 X, int32 Y, uint8& OutDirections, FDiaRoomData& RoomData) const
{
	for(const EDiaDirection& Dir : GetAllDirections())
	{
		int32 DirectionX = Dir == EDiaDirection::North ? 1 : (Dir == EDiaDirection::South ? -1 : 0);
		int32 DirectionY = Dir == EDiaDirection::East ? 1 : (Dir == EDiaDirection::West ? -1 : 0);

		int32 NextX = X + DirectionX;
		int32 NextY = Y + DirectionY;

		if(NextX >= 0 && NextX < MapWidth && NextY >= 0 && NextY < MapHeight)
		{
			int32 Index = GetIndex(NextX, NextY);
			if(MapData[GetIndex(NextX, NextY)].RoomID != NAME_None)
			{
				TArray<EDiaDirection> DestDirections = DiaMapGenerator::GetDirections(MapData[Index].Directions);
				for (const EDiaDirection& DestDir : DestDirections)
				{
					if (GetOppositeDirection(Dir) == DestDir)
					{
						OutDirections |= DiaMapGenerator::SetDirection(Dir);
					}
				}
			}
		}
	}

	CalcuateCorridorType(OutDirections, RoomData.RotateDegree, RoomData.RoomID);
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
		// North=+X, East=+Y 좌표계 기준 DefaultShapeBit
		// 직선: 메시가 ±X로 열림 → N|S = 0b0101
		// ㄴ자: 메시가 서-북으로 열림 → W|N = 0b1001. 메시 기준 Yaw +90° 보정.
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
		// T자: 메시가 -X, +Y, -Y로 열림 → S|E|W = 0b1110. 메시 기준 Yaw -90° 보정.
		const uint8 DefaultShapeBit = 0b1110;
		OutRoomID = TEXT("TCorridor");
		CalcuateCorridorDegree(DefaultShapeBit, Directions, OutRotateDegree);
		OutRotateDegree = (OutRotateDegree - 90 + 360) % 360;
	}
	break;
	case 4:
		//Cross모양 return;
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
	int32 MaxRooms = MapWidth * MapHeight; // 최대 방 수는 맵 크기에 따라 결정
	float TotalWeight = 0.f;
	for (const FRoomWeight& Entry : Rule.CandidateWeights)
	{
		// 최대 방 수를 초과한 경우 End가 아닌 방은 후보에서 제외
		if (CurrentRooms >= MaxRooms && Entry.RoomID != TEXT("End"))
		{
			continue;
		}
		//최소 방 개수가 안된 경우 END 호출 금지
		if(CurrentRooms < MinMapRoomCount && Entry.RoomID == TEXT("End"))
		{
			continue;
		}
		

		TotalWeight += Entry.Weight;
	}

	if (TotalWeight <= 0.f) return;

	float RandomValue = FMath::FRandRange(0.f, TotalWeight);
	float Acc = 0.f;
	FName ChosenID = NAME_None;
	for (const FRoomWeight& Entry : Rule.CandidateWeights)
	{
		// 최대 방 수를 초과한 경우 End가 아닌 방은 후보에서 제외
		if (CurrentRooms >= MaxRooms && Entry.RoomID != TEXT("End"))
		{
			continue;
		}
		//최소 방 개수가 안된 경우 END 호출 금지
		if (CurrentRooms < MinMapRoomCount && Entry.RoomID == TEXT("End"))
		{
			continue;
		}

		Acc += Entry.Weight;
		if (RandomValue <= Acc)
		{
			ChosenID = Entry.RoomID;
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

