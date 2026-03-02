// Fill out your copyright notice in the Description page of Project Settings.


#include "Map/DiaRoomBase.h"
#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
#include "Types/MapGenerate.h"
#include "System/MonsterSpawnSubSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogARPG_Room, Log, All);

ADiaRoomBase::ADiaRoomBase()
{
	PrimaryActorTick.bCanEverTick = false;

	PackedLevelChildActorComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("RootScene"));
	SetRootComponent(PackedLevelChildActorComponent);

	// 동·북·서·남 방향으로 HalfTileSize만큼 (타일 크기는 MapGenerate.h DiaMapConstants)
	const float H = DiaMapConstants::HalfTileSize;
	const FVector TriggerOffsets[] = {
		FVector(H, 0.f, 150.f),
		FVector(0.f, H, 150.f),
		FVector(-H, 0.f, 150.f),
		FVector(0.f, -H, 150.f)
	};
	for (int32 i = 0; i < 4; ++i)
	{
		FString TriggerName = FString::Printf(TEXT("RoomEnterTrigger%d"), i);
		UBoxComponent* Trigger = CreateDefaultSubobject<UBoxComponent>(*TriggerName);
		Trigger->SetupAttachment(RootComponent);
		Trigger->SetCollisionProfileName(TEXT("RoomEnterTrigger"));
		Trigger->SetBoxExtent(FVector(200.f, 200.f, 100.f));
		Trigger->SetRelativeLocation(TriggerOffsets[i]);
		RoomEnterTriggers.Add(Trigger);
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
			Trigger->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnRoomEnterTriggerOverlap);
		}
	}
}

// Called every frame
void ADiaRoomBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADiaRoomBase::OnRoomEnterTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogARPG_Room, Log, TEXT("ADiaRoomBase::OnRoomEnterTriggerOverlap: Actor %s entered room at location %s"), *OtherActor->GetName(), *GetActorLocation().ToString());
	UMonsterSpawnSubSystem* SpawnSubsystem = GetWorld()->GetSubsystem<UMonsterSpawnSubSystem>();
	if (!IsValid(SpawnSubsystem))
	{
		UE_LOG(LogARPG_Room, Warning, TEXT("ADiaRoomBase::OnRoomEnterTriggerOverlap: Failed to get UMonsterSpawnSubSystem"));
		return;
	}
	for (UBoxComponent* Trigger : RoomEnterTriggers)
	{
		if (Trigger)
		{
			Trigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	// Actor 중앙 기준 반경 HalfTileSize 안에서 스폰
	UE_LOG(LogARPG_Room, Log, TEXT("Spawning monsters for group %s at location %s"), *SpawnGroup.ToString(), *GetActorLocation().ToString());
	SpawnSubsystem->SpawnMonsterGroup(SpawnGroup, GetActorLocation(), DiaMapConstants::HalfTileSize);

}

void ADiaRoomBase::CreateRoomMonsters()
{
}

void ADiaRoomBase::OnBattleStart()
{
}

void ADiaRoomBase::OnBattleEnd()
{
}

