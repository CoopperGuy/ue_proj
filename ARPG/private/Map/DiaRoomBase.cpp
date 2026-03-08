// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/DiaRoomBase.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/ArrowComponent.h"

#include "Types/MapGenerate.h"
#include "System/MonsterManager.h"

#include "Monster/DiaMonster.h"

#include "System/MonsterSpawnSubSystem.h"

//문은 blueprint에서 만들어놓고 붙인다.

DEFINE_LOG_CATEGORY_STATIC(LogARPG_Room, Log, All);

ADiaRoomBase::ADiaRoomBase()
{
	PrimaryActorTick.bCanEverTick = false;

	PackedLevelChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("RootScene"));
	SetRootComponent(PackedLevelChildActorComponent);

	// North=+X, East=+Y, South=-X, West=-Y (Unreal 좌표계 기준)
	const float H = DiaMapConstants::HalfTileSize;
	const FVector TriggerOffsets[] = {
		FVector(H, 0.f, 150.f),    // North (+X = Forward)
		FVector(0.f, H, 150.f),    // East  (+Y = Right)
		FVector(-H, 0.f, 150.f),   // South (-X = Backward)
		FVector(0.f, -H, 150.f)    // West  (-Y = Left)
	};
	for (int32 i = 0; i < 4; ++i)
	{
		FString TriggerName = FString::Printf(TEXT("RoomEnterTrigger%d"), i);
		UBoxComponent* Trigger = CreateDefaultSubobject<UBoxComponent>(*TriggerName);
		Trigger->SetupAttachment(RootComponent);
		Trigger->SetCollisionProfileName(TEXT("RoomEnterTrigger"));
		Trigger->SetBoxExtent(FVector(10.f, 200.f, 100.f));
		Trigger->SetRelativeLocation(TriggerOffsets[i]);
		Trigger->SetRelativeRotation(FRotator(0.f, i * 90.f, 0.f));
		RoomEnterTriggers.Add(Trigger);

		FString ArrowName = FString::Printf(TEXT("DoorSpawnPoint%d"), i);
		UArrowComponent* Arrow = CreateDefaultSubobject<UArrowComponent>(*ArrowName);
		Arrow->SetupAttachment(RootComponent);
		Arrow->SetRelativeLocation(TriggerOffsets[i]);
		Arrow->SetRelativeRotation(FRotator(0.f, i * 90.f, 0.f));
		DoorSpawnPoints.Add(Arrow);
	}
}

// Called when the game starts or when spawned
void ADiaRoomBase::BeginPlay()
{
	Super::BeginPlay();
	
	for (UBoxComponent* Trigger : RoomEnterTriggers)
	{
		if (Trigger && TileType == ETileType::Floor)
		{
			Trigger->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnRoomEnterTriggerEndOverlap);
		}
	}

}

void ADiaRoomBase::OnConstruction(const FTransform& Transform)
{
	// DoorSpawnPoint(Arrow)를 북·동·남·서 순으로 각 방향을 바라보게 설정 (스폰 시점에 확실히 적용)
	for(int32 i = 0; i < RoomEnterTriggers.Num(); ++i)
	{
		if (UBoxComponent* Trigger = RoomEnterTriggers[i])
		{
			Trigger->SetRelativeRotation(FRotator(0.f, i * 90.f, 0.f));
		}
	}

	for (int32 i = 0; i < DoorSpawnPoints.Num(); ++i)
	{
		if (UArrowComponent* Arrow = DoorSpawnPoints[i])
		{
			Arrow->SetRelativeRotation(FRotator(0.f, i * 90.f, 0.f));
		}
	}

	if (!IsValid(RoomDoorClass))
	{
		return;
	}

	//문이 스폰되는 위치를 체크하는 direction을 잡으려 하는데 잘 되지않음 좀 더 수정필요
	//arrowcomponent의 실제적 위치는 큰 상관없을거같음. 게임내에서 로직적으로 돌아가는그것이 중요할듯.
	
	float YawRotation = Transform.GetRotation().Rotator().Yaw;

	TArray<EDiaDirection> Directions = DiaMapGenerator::GetDirections(DiaMapGenerator::AllDirectionsBit);
	uint8 NativeDirections = DiaMapGenerator::RotateDirectionsDegree(-YawRotation, DoorDirections);
	TArray<EDiaDirection> RoomDirections = DiaMapGenerator::GetDirections(NativeDirections);

	for(int32 i = 0; i < 4; ++i)
	{
		int32 DirValue = static_cast<uint8>(Directions[i]);

		if (!IsValid(DoorSpawnPoints[DirValue]))
			continue;

		//해당 방위가 연동되어있는지 체크하는 함수.
		if (RoomDirections.Contains(Directions[i]))
		{
			RoomEnterTriggers[DirValue]->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			DoorSpawnPoints[DirValue]->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

			AActor* Door = GetWorld()->SpawnActor<AActor>(RoomDoorClass, DoorSpawnPoints[DirValue]->GetComponentTransform());
			if (Door)
			{
				RoomDoors.Add(Door);
				Door->SetActorHiddenInGame(true);
				Door->SetActorEnableCollision(false);
			}
		}
		else
		{
			RoomEnterTriggers[DirValue]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			DoorSpawnPoints[DirValue]->SetHiddenInGame(true);
			DoorSpawnPoints[DirValue]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			DoorSpawnPoints[DirValue]->SetComponentTickEnabled(false);
			DoorSpawnPoints[DirValue]->SetActive(false);
		}
	}
}

// Called every frame
void ADiaRoomBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADiaRoomBase::InitRoom()
{
	UMonsterSpawnSubSystem* SpawnSubsystem = GetWorld()->GetSubsystem<UMonsterSpawnSubSystem>();
	if (!IsValid(SpawnSubsystem))
	{
		UE_LOG(LogARPG_Room, Warning, TEXT("ADiaRoomBase::OnRoomEnterTriggerOverlap: Failed to get UMonsterSpawnSubSystem"));
		return;
	}
	const FMapSpawnInfo& MapSpawnInfo = SpawnSubsystem->GetSpawnInfo(SpawnGroup);
	MaxMonsterCount = MapSpawnInfo.MonsterSpawnInfos.Num();
}

void ADiaRoomBase::OnRoomEnterTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UMonsterSpawnSubSystem* SpawnSubsystem = GetWorld()->GetSubsystem<UMonsterSpawnSubSystem>();
	//if (!IsValid(SpawnSubsystem))
	//{
	//	UE_LOG(LogARPG_Room, Warning, TEXT("ADiaRoomBase::OnRoomEnterTriggerOverlap: Failed to get UMonsterSpawnSubSystem"));
	//	return;
	//}

	//SpawnSubsystem->OnMonsterGroupSpawned.BindLambda([this](const TArray<ADiaMonster*>& InSpawnedMonsters)
	//{
	//	this->SpawnedMonsters.Reset();
	//	for (ADiaMonster* Monster : InSpawnedMonsters)
	//	{
	//		this->SpawnedMonsters.Add(Monster);
	//	}

	//	this->bMonstersSpawned = true;
	//	this->bIsBattleActive = true;

	//	UGameInstance* GI = GetWorld()->GetGameInstance();
	//	if (!GI) return;

	//	UMonsterManager* MM = GI->GetSubsystem<UMonsterManager>();
	//	if (!MM) return;
	//	MM->SetSpawnedMonstersForRoom(this->GetRoomGuid(), this->SpawnedMonsters);

	//	for(ADiaMonster* Monster : InSpawnedMonsters)
	//	{
	//		if (Monster)
	//		{
	//			Monster->SetOwningRoom(this->GetRoomGuid());
	//		}
	//	}
	//	UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase: Monster group %d monsters"), InSpawnedMonsters.Num());
	//});

	//for (UBoxComponent* Trigger : RoomEnterTriggers)
	//{
	//	if (Trigger)
	//	{
	//		Trigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//	}
	//}
	//SpawnSubsystem->SpawnMonsterGroup(SpawnGroup, GetActorLocation(), DiaMapConstants::HalfTileSize);

	//OnBattleStart();
}

void ADiaRoomBase::OnRoomEnterTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	FVector PlayerLocation = OtherActor->GetActorLocation();
	FVector DoorLocation = OverlappedComponent->GetComponentLocation();
	FVector ToPlayer = PlayerLocation - DoorLocation;
	ToPlayer.Z = 0.f; // 수평 방향으로만 계산
	ToPlayer.Normalize();
	//Trigger의 ForwardVector는 방의 바깥을 향하도록 설정되어 있으므로, ToPlayer와 ForwardVector의 내적이 음수이면 플레이어가 방 안으로 들어오는 방향입니다.
	FVector TriggerForward = OverlappedComponent->GetForwardVector();
	float DotProduct = FVector::DotProduct(ToPlayer, TriggerForward);
	if (DotProduct > 0.f)
	{
		UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase::OnRoomEnterTriggerEndOverlap: Player is exiting the room, ignoring overlap event."));
		return;
	}
	else
	{
		UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase::OnRoomEnterTriggerEndOverlap: Player is entering the room, processing overlap event."));
	}


	UMonsterSpawnSubSystem* SpawnSubsystem = GetWorld()->GetSubsystem<UMonsterSpawnSubSystem>();
	if (!IsValid(SpawnSubsystem))
	{
		UE_LOG(LogARPG_Room, Warning, TEXT("ADiaRoomBase::OnRoomEnterTriggerOverlap: Failed to get UMonsterSpawnSubSystem"));
		return;
	}

	SpawnSubsystem->OnMonsterGroupSpawned.BindLambda([this](const TArray<ADiaMonster*>& InSpawnedMonsters)
		{
			this->SpawnedMonsters.Reset();
			for (ADiaMonster* Monster : InSpawnedMonsters)
			{
				this->SpawnedMonsters.Add(Monster);
			}

			this->bMonstersSpawned = true;
			this->bIsBattleActive = true;

			UGameInstance* GI = GetWorld()->GetGameInstance();
			if (!GI) return;

			UMonsterManager* MM = GI->GetSubsystem<UMonsterManager>();
			if (!MM) return;
			MM->SetSpawnedMonstersForRoom(this->GetRoomGuid(), this->SpawnedMonsters);

			for (ADiaMonster* Monster : InSpawnedMonsters)
			{
				if (Monster)
				{
					Monster->SetOwningRoom(this->GetRoomGuid());
				}
			}
			UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase: Monster group %d monsters"), InSpawnedMonsters.Num());
		});

	for (UBoxComponent* Trigger : RoomEnterTriggers)
	{
		if (Trigger)
		{
			Trigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	SpawnSubsystem->SpawnMonsterGroup(SpawnGroup, GetActorLocation(), DiaMapConstants::HalfTileSize);

	OnBattleStart();
}

void ADiaRoomBase::CreateRoomMonsters()
{
}

void ADiaRoomBase::OnBattleStart()
{
	//1. 몬스터 스폰
	//2. 문 닫기
	//3. 이동 막기 트리거 활성화
	//4. 플레이어에게 "Battle Start" UI 띄우기
	//5. 기타 등등..

	for (const auto& Door : RoomDoors)
	{
		if (Door)
		{
			Door->SetActorHiddenInGame(false);
			Door->SetActorEnableCollision(true);
		}
	}

	UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase::OnBattleEnd: Battle Started"));
}

void ADiaRoomBase::OnBattleEnd()
{
	for (const auto& Door : RoomDoors)
	{
		if (Door)
		{
			Door->SetActorHiddenInGame(true);
			Door->SetActorEnableCollision(false);
		}
	}

	//화면에 "Battle Clear" UI 띄우기, 플레이어가 방을 나갈 수 있도록 문 열기, 보상 등등..
	UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase::OnBattleEnd: Battle ended, all monsters defeated!"));
}

void ADiaRoomBase::RemoveRoomonster(ADiaMonster* Monster)
{
	if (!IsValid(Monster))
		return;
	SpawnedMonsters.Remove(Monster);
	--MaxMonsterCount;
	if(MaxMonsterCount <= 0)
	{
		OnBattleEnd();
	}
	UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase::Remove Roomonster: Monster removed, remaining count: %d"), MaxMonsterCount);
}
