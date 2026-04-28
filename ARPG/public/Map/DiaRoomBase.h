// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/DiaMonsterTable.h"
#include "Types/MapGenerate.h"
#include "DiaRoomBase.generated.h"

class ADiaMonster;
class UBoxComponent;
class USceneComponent;
class UArrowComponent;
class UChildActorComponent;

/// <summary>
/// Room은 현재 DataAsset으로 만들어지는데, 블루프린트로 만든후, 이 블루프린트를 DataAsset으로 만들어서 관리하는 방식으로 사용할 예정이다.
/// 몬스터에 대한 정보는 현재 만들어져있는 subsystem에서 가져올거고, 데이터도 다룰거다.
/// 구현 순서
/// 1. RoomEnterTriggerOverlap 이벤트 구현
/// 2. 몬스터 스폰 구현 (전투 시작 상태로 만든다)
/// 3. 몬스터가 모두 죽었는지 체크하는 로직 구현  (전투 종료 상태로 만든다)
/// 4. 전투 시작, 종료 이벤트 구현 (문 열고 닫는 로직 구현 및 보상)
/// </summary>
UCLASS()
class ARPG_API ADiaRoomBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ADiaRoomBase();

protected:
	virtual void BeginPlay() override;
	void ConfigurePackedLevelTraceCollision();

	void OnConstruction(const FTransform& Transform) override;
public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnRoomEnterTriggerOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnRoomEnterTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
	void CreateRoomMonsters();

	void OnBattleStart(const FGuid InGuid);
	void OnBattleEnd(const FGuid InGuid);
	bool IsMonsterGenerateRoom(ETileType _TileType);
protected:
	UPROPERTY(EditAnywhere)
	UChildActorComponent* PackedLevelChildActorComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TArray<UBoxComponent*> RoomEnterTriggers;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TSubclassOf<AActor> RoomDoorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TArray<UArrowComponent*> DoorSpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	TArray<AActor*> RoomDoors;

	UPROPERTY()
	uint8 DoorDirections = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Room")
	ETileType TileType;

	UPROPERTY()
	FGuid RoomGuid;

	UPROPERTY()
	ESpawnType SpawnType;

	bool isCleared = false;
public:
	FORCEINLINE void SetTileType(ETileType _TileType) { TileType = _TileType; }
	FORCEINLINE ETileType GetTileType() const { return TileType; }
	FORCEINLINE void SetRoomGuid(const FGuid& _RoomGuid) { RoomGuid = _RoomGuid; }
	FORCEINLINE const FGuid& GetRoomGuid() const { return RoomGuid; }
	FORCEINLINE void SetDoorDirections(uint8 _DoorDir) { DoorDirections = _DoorDir; }
	FORCEINLINE uint8 GetDoorDirections() const { return DoorDirections; }
	FORCEINLINE void SetSpawnType(ESpawnType _SpawnType) { SpawnType = _SpawnType; }
	FORCEINLINE ESpawnType GetSpawnType() const { return SpawnType; }
};
