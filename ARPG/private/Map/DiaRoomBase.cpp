// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/DiaRoomBase.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"

#include "Types/MapGenerate.h"
#include "System/MonsterManager.h"

#include "Monster/DiaMonster.h"

#include "DiaGameState.h"
#include "Logging/ARPGLogChannels.h"
//문은 blueprint에서 만들어놓고 붙인다.

ADiaRoomBase::ADiaRoomBase()
{
	PrimaryActorTick.bCanEverTick = false;

	PackedLevelChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("RootScene"));
	SetRootComponent(PackedLevelChildActorComponent);

	// North=+X, East=+Y, South=-X, West=-Y (Unreal 좌표계 기준)
	for (int32 i = 0; i < 4; ++i)
	{
		FString TriggerName = FString::Printf(TEXT("RoomEnterTrigger%d"), i);
		UBoxComponent* Trigger = CreateDefaultSubobject<UBoxComponent>(*TriggerName);
		Trigger->SetupAttachment(RootComponent);
		Trigger->SetCollisionProfileName(TEXT("RoomEnterTrigger"));
		RoomEnterTriggers.Add(Trigger);

		FString ArrowName = FString::Printf(TEXT("DoorSpawnPoint%d"), i);
		UArrowComponent* Arrow = CreateDefaultSubobject<UArrowComponent>(*ArrowName);
		Arrow->SetupAttachment(RootComponent);
		DoorSpawnPoints.Add(Arrow);
	}

	ConfigureDoorComponents();
}

void ADiaRoomBase::EnsureDoorComponentCount(int32 DesiredCount)
{
	DesiredCount = FMath::Max(0, DesiredCount);
	while (RoomEnterTriggers.Num() < DesiredCount)
	{
		const FString TriggerName = FString::Printf(TEXT("RoomEnterTrigger%d"), RoomEnterTriggers.Num());
		UBoxComponent* Trigger = NewObject<UBoxComponent>(this, *TriggerName);
		Trigger->SetupAttachment(RootComponent);
		Trigger->SetCollisionProfileName(TEXT("RoomEnterTrigger"));
		Trigger->RegisterComponent();
		AddInstanceComponent(Trigger);
		RoomEnterTriggers.Add(Trigger);
	}

	while (DoorSpawnPoints.Num() < DesiredCount)
	{
		const FString ArrowName = FString::Printf(TEXT("DoorSpawnPoint%d"), DoorSpawnPoints.Num());
		UArrowComponent* Arrow = NewObject<UArrowComponent>(this, *ArrowName);
		Arrow->SetupAttachment(RootComponent);
		Arrow->RegisterComponent();
		AddInstanceComponent(Arrow);
		DoorSpawnPoints.Add(Arrow);
	}
}

TArray<FDiaRoomPort> ADiaRoomBase::GetEffectivePossibleDoorPorts() const
{
	if (!PossibleDoorPorts.IsEmpty())
	{
		return PossibleDoorPorts;
	}

	return DiaMapGenerator::MakeDefaultPorts(RoomSize);
}

bool ADiaRoomBase::IsActiveDoorPort(const FDiaRoomPort& Port) const
{
	for (const FDiaRoomPort& ActivePort : ActiveDoorPorts)
	{
		if (ActivePort == Port)
		{
			return true;
		}
	}

	return false;
}

bool ADiaRoomBase::CanStartBattle() const
{
	return IsMonsterSpawnEnabled() && RoomBattleState == EDiaRoomBattleState::Idle && IsMonsterGenerateRoom(TileType);
}

void ADiaRoomBase::SetRoomBattleState(EDiaRoomBattleState NewState)
{
	RoomBattleState = NewState;
}

void ADiaRoomBase::SetRoomEnterTriggersEnabled(bool bEnabled)
{
	const bool bMonsterRoom = IsMonsterSpawnEnabled() && IsMonsterGenerateRoom(TileType);
	const bool bCanEnable = bEnabled && bMonsterRoom && RoomBattleState == EDiaRoomBattleState::Idle;
	const TArray<FDiaRoomPort> EffectivePorts = GetEffectivePossibleDoorPorts();

	for (int32 Index = 0; Index < RoomEnterTriggers.Num(); ++Index)
	{
		UBoxComponent* Trigger = RoomEnterTriggers[Index];
		if (!IsValid(Trigger))
		{
			continue;
		}

		const bool bActivePort = EffectivePorts.IsValidIndex(Index) && IsActiveDoorPort(EffectivePorts[Index]);
		Trigger->SetCollisionEnabled(bCanEnable && bActivePort ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void ADiaRoomBase::ConfigureDoorComponents()
{
	const FIntPoint SafeRoomSize(FMath::Max(1, RoomSize.X), FMath::Max(1, RoomSize.Y));
	const TArray<FDiaRoomPort> EffectivePorts = GetEffectivePossibleDoorPorts();
	EnsureDoorComponentCount(EffectivePorts.Num());

	for (int32 i = 0; i < RoomEnterTriggers.Num(); ++i)
	{
		const bool bHasPort = EffectivePorts.IsValidIndex(i);
		const FDiaRoomPort Port = bHasPort ? EffectivePorts[i] : FDiaRoomPort();
		const FIntPoint DirectionOffset = bHasPort ? DiaMapGenerator::GetDirectionOffset(Port.Direction) : FIntPoint::ZeroValue;
		const FIntPoint LocalCell(
			FMath::Clamp(Port.LocalCell.X, 0, SafeRoomSize.X - 1),
			FMath::Clamp(Port.LocalCell.Y, 0, SafeRoomSize.Y - 1));
		const float LocalX = (static_cast<float>(LocalCell.X) - (static_cast<float>(SafeRoomSize.X - 1) * 0.5f)) * DiaMapConstants::TileSize;
		const float LocalY = (static_cast<float>(LocalCell.Y) - (static_cast<float>(SafeRoomSize.Y - 1) * 0.5f)) * DiaMapConstants::TileSize;
		const FVector TriggerOffset(
			LocalX + static_cast<float>(DirectionOffset.X) * DiaMapConstants::HalfTileSize,
			LocalY + static_cast<float>(DirectionOffset.Y) * DiaMapConstants::HalfTileSize,
			150.f);
		const float Yaw = Port.Direction == EDiaDirection::None ? 0.f : static_cast<float>(static_cast<uint8>(Port.Direction)) * 90.f;

		if (UBoxComponent* Trigger = RoomEnterTriggers[i])
		{
			Trigger->SetBoxExtent(FVector(10.f, 200.f, 100.f));
			Trigger->SetRelativeLocation(TriggerOffset);
			Trigger->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
			Trigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (UArrowComponent* Arrow = DoorSpawnPoints[i])
		{
			Arrow->SetRelativeLocation(TriggerOffset);
			Arrow->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
			Arrow->SetHiddenInGame(!bHasPort);
			Arrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Arrow->SetComponentTickEnabled(false);
			Arrow->SetActive(bHasPort);
		}
	}
}

// Called when the game starts or when spawned
void ADiaRoomBase::BeginPlay()
{
	Super::BeginPlay();

	for (UBoxComponent* Trigger : RoomEnterTriggers)
	{
		if (Trigger && IsMonsterSpawnEnabled() && IsMonsterGenerateRoom(TileType))
		{
			Trigger->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnRoomEnterTriggerEndOverlap);
		}
	}

	ADiaGameState* DiaGameState = GetWorld() ? GetWorld()->GetGameState<ADiaGameState>() : nullptr;
	if (IsValid(DiaGameState))
	{
		DiaGameState->OnRoomCleared.AddUObject(this, &ThisClass::OnBattleEnd);
		DiaGameState->OnRoomBattleStart.AddUObject(this, &ThisClass::OnBattleStart);
	}

}



void ADiaRoomBase::OnConstruction(const FTransform& Transform)
{
	ConfigureDoorComponents();

	for (AActor* Door : RoomDoors)
	{
		if (IsValid(Door))
		{
			Door->Destroy();
		}
	}
	RoomDoors.Reset();

	for (AActor* Door : ClosedPortDoors)
	{
		if (IsValid(Door))
		{
			Door->Destroy();
		}
	}
	ClosedPortDoors.Reset();

	const bool bMonsterRoom = IsMonsterSpawnEnabled() && IsMonsterGenerateRoom(TileType);
	const TArray<FDiaRoomPort> EffectivePorts = GetEffectivePossibleDoorPorts();
	for (int32 Index = 0; Index < EffectivePorts.Num(); ++Index)
	{
		if (!DoorSpawnPoints.IsValidIndex(Index) || !IsValid(DoorSpawnPoints[Index]))
		{
			continue;
		}

		const bool bActivePort = IsActiveDoorPort(EffectivePorts[Index]);
		if (RoomEnterTriggers.IsValidIndex(Index) && IsValid(RoomEnterTriggers[Index]))
		{
			const bool bEnableTrigger = bActivePort && bMonsterRoom && RoomBattleState == EDiaRoomBattleState::Idle;
			RoomEnterTriggers[Index]->SetCollisionEnabled(bEnableTrigger ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		}

		if (!IsValid(RoomDoorClass))
		{
			continue;
		}

		AActor* Door = GetWorld()->SpawnActor<AActor>(RoomDoorClass, DoorSpawnPoints[Index]->GetComponentTransform());
		if (!Door)
		{
			continue;
		}

		if (bActivePort && bMonsterRoom)
		{
			RoomDoors.Add(Door);
			Door->SetActorHiddenInGame(true);
			Door->SetActorEnableCollision(false);
		}
		else if (!bActivePort && bMonsterRoom)
		{
			ClosedPortDoors.Add(Door);
			Door->SetActorHiddenInGame(false);
			Door->SetActorEnableCollision(true);
		}
		else
		{
			Door->Destroy();
		}
	}
}

void ADiaRoomBase::SetDoorPorts(const TArray<FDiaRoomPort>& InPossibleDoorPorts, const TArray<FDiaRoomPort>& InActiveDoorPorts)
{
	PossibleDoorPorts = InPossibleDoorPorts.IsEmpty() ? DiaMapGenerator::MakeDefaultPorts(RoomSize) : InPossibleDoorPorts;
	ActiveDoorPorts.Reset();
	for (const FDiaRoomPort& ActivePort : InActiveDoorPorts)
	{
		if (PossibleDoorPorts.Contains(ActivePort))
		{
			ActiveDoorPorts.AddUnique(ActivePort);
		}
	}
	ConfigureDoorComponents();
}

// Called every frame
void ADiaRoomBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase::OnRoomEnterTriggerEndOverlap: Overlap ended with actor %s"), *GetNameSafe(OtherActor));
	if (!CanStartBattle())
		return;

	const APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController || OtherActor != PlayerController->GetPawn())
	{
		return;
	}

	if (!IsValid(OverlappedComponent))
	{
		return;
	}

	FVector PlayerLocation = OtherActor->GetActorLocation();
	FVector DoorLocation = OverlappedComponent->GetComponentLocation();
	FVector ToPlayer = PlayerLocation - DoorLocation;
	ToPlayer.Z = 0.f; // 수평 방향으로만 계산
	if (!ToPlayer.Normalize())
	{
		return;
	}
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


	ADiaGameState* DiaGameState = GetWorld()->GetGameState<ADiaGameState>();

	if (DiaGameState)
	{
		SetRoomBattleState(EDiaRoomBattleState::BattleActive);
		SetRoomEnterTriggersEnabled(false);

		if (!DiaGameState->SpawnRoomMonsters(this->GetRoomGuid(), GetActorLocation(), GetTileType(), SpawnType, DiaMapConstants::HalfTileSize))
		{
			SetRoomBattleState(EDiaRoomBattleState::Idle);
			SetRoomEnterTriggersEnabled(true);
		}
	}
	else
	{
		UE_LOG(LogARPG_Room, Warning, TEXT("ADiaRoomBase::OnRoomEnterTriggerEndOverlap: Failed to get ADiaGameState"));
	}

	//UMonsterSpawnSubSystem* SpawnSubsystem = GetWorld()->GetSubsystem<UMonsterSpawnSubSystem>();
	//if (!IsValid(SpawnSubsystem))
	//{
	//	UE_LOG(LogARPG_Room, Warning, TEXT("ADiaRoomBase::OnRoomEnterTriggerOverlap: Failed to get UMonsterSpawnSubSystem"));
	//	return;
	//}

	//SpawnSubsystem->OnMonsterGroupSpawned.BindLambda([this](const TArray<ADiaMonster*>& InSpawnedMonsters)
	//	{
	//		this->SpawnedMonsters.Reset();
	//		for (ADiaMonster* Monster : InSpawnedMonsters)
	//		{
	//			this->SpawnedMonsters.Add(Monster);
	//		}

	//		this->bMonstersSpawned = true;
	//		this->bIsBattleActive = true;

	//		UGameInstance* GI = GetWorld()->GetGameInstance();
	//		if (!GI) return;

	//		UMonsterManager* MM = GI->GetSubsystem<UMonsterManager>();
	//		if (!MM) return;
	//		MM->SetSpawnedMonstersForRoom(this->GetRoomGuid(), this->SpawnedMonsters);

	//		for (ADiaMonster* Monster : InSpawnedMonsters)
	//		{
	//			if (Monster)
	//			{
	//				Monster->SetOwningRoom(this->GetRoomGuid());
	//			}
	//		}
	//		UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase: Monster group %d monsters"), InSpawnedMonsters.Num());
	//	});

	//for (UBoxComponent* Trigger : RoomEnterTriggers)
	//{
	//	if (Trigger)
	//	{
	//		Trigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//	}
	//}
	//SpawnSubsystem->SpawnMonsterGroup(SpawnGroup, GetActorLocation(), DiaMapConstants::HalfTileSize);
}

void ADiaRoomBase::CreateRoomMonsters()
{
}

void ADiaRoomBase::SetRoomRole(EDiaRoomRole NewRole)
{
	RoomRole = NewRole;
	if (RoomRole == EDiaRoomRole::Start)
	{
		SetRoomBattleState(EDiaRoomBattleState::Cleared);
		SetRoomEnterTriggersEnabled(false);

		for (AActor* Door : RoomDoors)
		{
			if (IsValid(Door))
			{
				Door->SetActorHiddenInGame(true);
				Door->SetActorEnableCollision(false);
			}
		}
		return;
	}

	SetRoomEnterTriggersEnabled(true);
}

bool ADiaRoomBase::IsMonsterSpawnEnabled() const
{
	return RoomRole != EDiaRoomRole::Start;
}

void ADiaRoomBase::OnBattleStart(const FGuid InGuid)
{
	if(InGuid != GetRoomGuid())
	{
		return;
	}
	//1. 몬스터 스폰
	//2. 문 닫기
	//3. 이동 막기 트리거 활성화
	//4. 플레이어에게 "Battle Start" UI 띄우기
	//5. 기타 등등..

	SetRoomBattleState(EDiaRoomBattleState::BattleActive);
	SetRoomEnterTriggersEnabled(false);

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

void ADiaRoomBase::OnBattleEnd(const FGuid InGuid)
{
	if (InGuid != GetRoomGuid())
	{
		return;
	}

	SetRoomBattleState(EDiaRoomBattleState::Cleared);
	SetRoomEnterTriggersEnabled(false);

	for (const auto& Door : RoomDoors)
	{
		if (Door)
		{
			Door->SetActorHiddenInGame(true);
			Door->SetActorEnableCollision(false);
		}
	}
}

bool ADiaRoomBase::IsMonsterGenerateRoom(ETileType _TileType) const
{
	if (_TileType != ETileType::Corridor && _TileType != ETileType::Empty)
		return true;
	return false;
}

void ADiaRoomBase::SetRoomSize(const FIntPoint& InRoomSize)
{
	RoomSize = FIntPoint(FMath::Max(1, InRoomSize.X), FMath::Max(1, InRoomSize.Y));
	ConfigureDoorComponents();
}
