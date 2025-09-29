// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MapInfoSubsystem.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Kismet/GameplayStatics.h"

#include "System/MonsterSpawnSubSystem.h"


FVector GetLevelCenterFromNavMesh(UWorld* World)
{
    if (!World)
        return FVector::ZeroVector;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavSys)
        return FVector::ZeroVector;

    ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
    ARecastNavMesh* NavMesh = Cast<ARecastNavMesh>(NavData);

    // 1) NavMesh 바운즈가 유효하면 그 중심 사용
    if (NavMesh)
    {
        const FBox NavBounds = NavMesh->GetNavMeshBounds();
        const FVector Center = NavBounds.GetCenter();
        const FVector Extent = NavBounds.GetExtent();
        if (Extent.SizeSquared() > KINDA_SMALL_NUMBER)
        {
            return Center;
        }
    }

    // 2) 플레이어 주변을 우선 시드로 사용하여 NavMesh로 투영
    const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    const FVector Seed = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    FNavLocation Projected;
    const FVector QueryExtent(2000.f, 2000.f, 4000.f);
    if (NavSys->ProjectPointToNavigation(Seed, Projected, QueryExtent, NavData, nullptr))
    {
        return Projected.Location;
    }

    // 3) 주변 그리드 오프셋으로 재시도
    static const FVector Offsets[] = {
        FVector(1000, 0, 0), FVector(-1000, 0, 0), FVector(0, 1000, 0), FVector(0, -1000, 0),
        FVector(1000, 1000, 0), FVector(-1000, 1000, 0), FVector(1000, -1000, 0), FVector(-1000, -1000, 0)
    };
    for (const FVector& Offset : Offsets)
    {
        const FVector Test = Seed + Offset;
        if (NavSys->ProjectPointToNavigation(Test, Projected, QueryExtent, NavData, nullptr))
        {
            return Projected.Location;
        }
    }

    // 4) 실패 시 0 반환(상위에서 재시도/지연 처리됨)
    UE_LOG(LogTemp, Warning, TEXT("[MapInfoSubsystem] GetLevelCenterFromNavMesh failed. Seed=%s"), *Seed.ToString());
    return FVector::ZeroVector;
}

void UMapInfoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadMapInfo();
}

void UMapInfoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

//맵 스폰 데이터 로드
void UMapInfoSubsystem::CreateMapSpawnData()
{
	UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] CreateMapSpawnData 시작"));
	
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[MapInfoSubsystem] 유효한 World 객체가 없습니다"));
		return;
	}
	
	UMonsterSpawnSubSystem* SpawnSystem = World->GetSubsystem<UMonsterSpawnSubSystem>();
	if (!IsValid(SpawnSystem))
	{
		UE_LOG(LogTemp, Error, TEXT("[MapInfoSubsystem] MonsterSpawnSubSystem을 찾을 수 없습니다"));
		return;
	}
	
	// 원본 맵 이름 가져오기
	FString CleanMapName = UGameplayStatics::GetCurrentLevelName(World);
	UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] GameplayStatics 사용 맵 이름: %s"), *CleanMapName);
	FName MapName = FName(*CleanMapName);
	
	FMapInfoTable* MapInfo = MapInfoCache.Find(MapName);
	if (MapInfo)
	{
		FVector CenterPos = GetLevelCenterFromNavMesh(World);
		UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] 맵 정보 찾음 - MapID: %s, 중심점: (%f, %f, %f), 반경: %f"),
			*MapInfo->MapID.ToString(), CenterPos.X, CenterPos.Y, CenterPos.Z, MapInfo->MpaRadius);
		
		SpawnSystem->SpawnMonsterGroup(MapInfo->MapID, CenterPos, MapInfo->MpaRadius);
		UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] 몬스터 그룹 스폰 요청 완료 - GroupID: %s"), *MapInfo->MapID.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[MapInfoSubsystem] 맵 '%s'에 대한 정보를 찾을 수 없습니다. 캐시된 맵 개수: %d"), 
			*MapName.ToString(), MapInfoCache.Num());
		
		// 디버깅: 캐시된 모든 맵 이름 출력
		UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] 캐시된 맵 목록:"));
		for (const auto& Pair : MapInfoCache)
		{
			UE_LOG(LogTemp, Log, TEXT("  - %s"), *Pair.Key.ToString());
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] CreateMapSpawnData 종료"));
}

void UMapInfoSubsystem::LoadMapInfo()
{
	UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] LoadMapInfo 시작 - 데이터 테이블 경로: %s"), *MapInfoDataTablePath);
	
	// 데이터 테이블 로드
	MapInfoDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *MapInfoDataTablePath));
	if (!MapInfoDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[MapInfoSubsystem] 맵 정보 데이터 테이블을 로드하지 못했습니다: %s"), *MapInfoDataTablePath);
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] 데이터 테이블 로드 성공"));
	
	// 데이터 테이블에서 모든 행을 가져와서 캐시에 저장
	MapInfoCache.Empty();
	TArray<FName> RowNames = MapInfoDataTable->GetRowNames();
	UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] 데이터 테이블에서 %d개의 행을 찾았습니다"), RowNames.Num());
	
	int32 SuccessCount = 0;
	for (const FName& RowName : RowNames)
	{
		FMapInfoTable* MapInfoRow = MapInfoDataTable->FindRow<FMapInfoTable>(RowName, TEXT(""));
		if (MapInfoRow)
		{
			UE_LOG(LogTemp, Verbose, TEXT("[MapInfoSubsystem] 행 '%s' 로드 - MapID: %s, 이름: %s, 레벨: %d-%d"),
				*RowName.ToString(), *MapInfoRow->MapID.ToString(), 
				*MapInfoRow->DisplayName.ToString(), MapInfoRow->MinLevel, MapInfoRow->MaxLevel);
			
			MapInfoCache.Add(MapInfoRow->MapID, *MapInfoRow);
			SuccessCount++;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[MapInfoSubsystem] 행 '%s'의 데이터를 읽지 못했습니다"), *RowName.ToString());
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] 총 %d개의 맵 정보를 캐시에 저장했습니다 (성공: %d, 실패: %d)"), 
		MapInfoCache.Num(), SuccessCount, RowNames.Num() - SuccessCount);
	
	UE_LOG(LogTemp, Log, TEXT("[MapInfoSubsystem] LoadMapInfo 종료"));
}

