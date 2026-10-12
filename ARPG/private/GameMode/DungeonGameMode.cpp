// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DungeonGameMode.h"
#include "Character/DiaCharacter.h"
#include "Controller/DiaController.h"
#include "Actor/DiaPortal.h"
#include "Actor/DiaRoomRewardActor.h"

#include "NavigationSystem.h"

#include "DiaGameState.h"

#include "Map/DiaRoomBase.h"

#include "Item/DiaItem.h"

#include "UI/HUDWidget.h"
#include "UI/Inventory/MainInventory.h"
#include "Blueprint/UserWidget.h"

#include "System/MapInfoSubsystem.h"
#include "System/DiaMapGeneratorSubsystem.h"
#include "System/ItemSubsystem.h"
#include "System/DiaRewardSubsystem.h"
#include "DiaComponent/DiaSkillManagerComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/ARPGLogChannels.h"

ADungeonGameMode::ADungeonGameMode()
{
	// 기본 클래스 설정
	DefaultPawnClass = ADiaCharacter::StaticClass();
	GameStateClass = ADiaGameState::StaticClass();

	// 기본 HUD 위젯 클래스 설정 (블루프린트에서 오버라이드 가능)
	// Blueprint 생성 클래스는 이름 뒤에 _C 접미사가 붙음
	static ConstructorHelpers::FClassFinder<UHUDWidget> HUDWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_HUDWidget.WBP_HUDWidget_C"));
	if (HUDWidgetClassFinder.Succeeded())
	{
		HUDWidgetClass = HUDWidgetClassFinder.Class;
	}
}

void ADungeonGameMode::BeginPlay()
{
	Super::BeginPlay();
    if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        NavSys->Build(); // 강제 리빌드
    }

	//subsystem 초기화
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UMapInfoSubsystem* MapInfo = GI->GetSubsystem<UMapInfoSubsystem>();

	ADiaGameState* DiaGameState = Cast<ADiaGameState>(GameState);
	if (IsValid(DiaGameState))
	{
		DiaGameState->OnRoonStateChange.AddUObject(this, &ADungeonGameMode::OnRoonStateChange);
	}

	//if (IsValid(MapInfo))
	//{
	//	MapInfo->CreateMapSpawnData();
	//}
}

void ADungeonGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ADungeonGameMode::OnRoonStateChange(FGuid RoomID, EDiaRoomBattleState NewState)
{
	if (NewState == EDiaRoomBattleState::Cleared)
	{
		OnRoomCleared(RoomID);
	}
}

void ADungeonGameMode::OnRoomCleared(FGuid RoomID)
{
    if (UDiaMapGeneratorSubsystem* Map = GetWorld()->GetSubsystem<UDiaMapGeneratorSubsystem>())
        PresentRoomReward(Map->GetRoomActor(RoomID));
}

void ADungeonGameMode::InitializeRoomRewards()
{
    const UDiaMapGeneratorSubsystem* Map = GetWorld()->GetSubsystem<UDiaMapGeneratorSubsystem>();
    const ADiaGameState* GS = GetWorld()->GetGameState<ADiaGameState>();
    if (!Map || !GS) return;
    for (const FDiaRoomData& Data : Map->GetRoomLayout())
    {
        if (!Data.bIsAnchor || Data.RoomID.IsNone()) continue;
        ADiaRoomBase* Room = Map->GetRoomActor(Data.RoomGuid);
        if (Room && (Room->GetRoomRole() == EDiaRoomRole::Treasure || GS->IsRoomCleared(Data.RoomGuid)))
            PresentRoomReward(Room);
    }
}

void ADungeonGameMode::PresentRoomReward(ADiaRoomBase* Room)
{
    if (!Room || Room->IsStartRoom() || !Room->IsMonsterGenerateRoom(Room->GetTileType())) return;
    const FGuid ID = Room->GetRoomGuid();
    if (IsValid(RewardActors.FindRef(ID))) return;
    UDiaRewardSubsystem* Rewards = GetGameInstance()->GetSubsystem<UDiaRewardSubsystem>();
    if (!Rewards) return;
    const EDiaRoomRewardKind Kind = Room->IsStageClearRoom() ? EDiaRoomRewardKind::Boss
        : Room->GetRoomRole() == EDiaRoomRole::Treasure ? EDiaRoomRewardKind::Treasure : EDiaRoomRewardKind::Normal;
    const FDiaRoomRewardState* State = Rewards->FindRoomReward(ID);
    if (Kind != EDiaRoomRewardKind::Treasure)
        State = Rewards->ResolveRoomReward(ID, Kind, Cast<ADiaController>(GetWorld()->GetFirstPlayerController()));
    if (State && ((State->bClaimed && Kind != EDiaRoomRewardKind::Boss) || (State->bResolved && State->Options.IsEmpty()))) return;

    const FTransform Transform(FRotator::ZeroRotator, Room->GetActorLocation() + FVector(0.f, 0.f, 70.f));
    ADiaRoomRewardActor* Actor = GetWorld()->SpawnActorDeferred<ADiaRoomRewardActor>(
        ADiaRoomRewardActor::StaticClass(), Transform, Room, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (Actor)
    {
        Actor->InitializeReward(ID, Kind);
        Actor->FinishSpawning(Transform);
        RewardActors.Add(ID, Actor);
    }
}

void ADungeonGameMode::OnRoomRewardClaimed(FGuid RoomID)
{
    UDiaRewardSubsystem* Rewards = GetGameInstance()->GetSubsystem<UDiaRewardSubsystem>();
    const FDiaRoomRewardState* State = Rewards ? Rewards->FindRoomReward(RoomID) : nullptr;
    if (!State || !State->bClaimed) return;
    ADiaRoomRewardActor* Actor = RewardActors.FindRef(RoomID);
    if (State->Kind == EDiaRoomRewardKind::Boss)
    {
        if (Actor) Actor->InitializeReward(RoomID, State->Kind);
        if (ADiaController* Controller = Cast<ADiaController>(GetWorld()->GetFirstPlayerController()))
            Controller->ShowSessionResult(true);
        if (!AnnouncedStageRooms.Contains(RoomID))
        {
            AnnouncedStageRooms.Add(RoomID);
            if (ADiaGameState* GS = GetWorld()->GetGameState<ADiaGameState>()) GS->OnStageCleared.Broadcast(RoomID);
        }
    }
    else
    {
        if (Actor) Actor->Destroy();
        RewardActors.Remove(RoomID);
    }
}

void ADungeonGameMode::SpawnItemAtLocation(AActor* SpawnActor, const FItemBase& ItemData, int32 Level, int32 Quantity)
{
	if (IsValid(SpawnActor))
	{
		// 아이템 스폰
		FVector SpawnLocation = SpawnActor->GetActorLocation();
		SpawnLocation.Z += 100.f;
		FTransform SpawnTransform = SpawnActor->GetTransform();
		SpawnTransform.SetLocation(SpawnLocation);

		ADiaItem* SpawnedItem = GetWorld()->SpawnActorDeferred<ADiaItem>(ADiaItem::StaticClass(),
			 SpawnTransform, SpawnActor);

		UE_LOG(LogARPG, Warning, TEXT("Spawned Item at Location: %s"), *SpawnLocation.ToString());

		if (SpawnedItem)
		{
			SpawnedItem->SetItemProperty(ItemData, Level, Quantity);
			SpawnedItem->FinishSpawning(SpawnTransform);
			SpawnedItem->DropItem(ItemData.ItemID);
		}
	}
}

void ADungeonGameMode::SpawnInventoryItemAtLocation(AActor* SpawnActor, const FInventorySlot& InventoryItem)
{
	if (IsValid(SpawnActor))
	{
		FVector SpawnLocation = SpawnActor->GetActorLocation();
		SpawnLocation.Z += 100.f;
		FTransform SpawnTransform = SpawnActor->GetTransform();
		SpawnTransform.SetLocation(SpawnLocation);

		ADiaItem* SpawnedItem = GetWorld()->SpawnActorDeferred<ADiaItem>(ADiaItem::StaticClass(),
			SpawnTransform, SpawnActor);

		UE_LOG(LogARPG, Warning, TEXT("Spawned Inventory Item at Location: %s"), *SpawnLocation.ToString());

		if (SpawnedItem)
		{
			SpawnedItem->SetInventoryItem(InventoryItem);
			SpawnedItem->FinishSpawning(SpawnTransform);
			SpawnedItem->DropItem(InventoryItem.ItemInstance.ItemID);
		}
	}
}

void ADungeonGameMode::WarpOtherLevel(const FName& LevelName)
{
	UGameplayStatics::OpenLevel(GetWorld(), LevelName);

}

void ADungeonGameMode::HandlePortalInteraction(ADiaPortal* Portal, APlayerController* InteractingController)
{
	if (!IsValid(Portal) || !IsValid(InteractingController))
	{
		return;
	}

	const FName DestinationLevel = Portal->GetDestinationLevel();
	if (DestinationLevel.IsNone())
	{
		UE_LOG(LogARPG, Warning, TEXT("Portal %s has no DestinationLevel."),
			*GetNameSafe(Portal));
		return;
	}

	WarpOtherLevel(DestinationLevel);
}

UHUDWidget* const ADungeonGameMode::GetHUDWidget() const
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	ADiaController* DiaPC = Cast<ADiaController>(PC);
	if (!DiaPC)
	{
		UE_LOG(LogARPG, Warning, TEXT("ADungeonGameMode::GetHUDWidget - DiaController is null"));
		return nullptr;
	}

	// 실제 HUD는 컨트롤러가 관리
	return DiaPC->GetHUDWidget();

}
