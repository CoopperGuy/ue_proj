// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DungeonGameMode.h"
#include "Character/DiaCharacter.h"
#include "Controller/DiaController.h"

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
		DiaGameState->OnRoomCleared.AddUObject(this, &ADungeonGameMode::OnRoomCleared);
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

void ADungeonGameMode::OnRoomCleared(FGuid RoomID)
{
	UDiaMapGeneratorSubsystem* MapInfo = GetWorld()->GetSubsystem<UDiaMapGeneratorSubsystem>();
	if (!IsValid(MapInfo))
		return;

	ADiaRoomBase* RoomBase = MapInfo->GetRoomActor(RoomID);
	//보스룸 클리어 하면 클리어 했다고 보내야함.
	if (IsValid(RoomBase) && RoomBase->GetTileType() == ETileType::Boss)
	{
		ADiaGameState* DiaGameState = Cast<ADiaGameState>(GameState);
		if (IsValid(DiaGameState))
		{
			DiaGameState->ClearCurrentLevel();
		}
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
