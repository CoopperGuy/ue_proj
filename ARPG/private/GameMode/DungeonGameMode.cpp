// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DungeonGameMode.h"
#include "Character/DiaCharacter.h"
#include "Controller/DiaController.h"

#include "NavigationSystem.h"

#include "Item/DiaItem.h"

#include "UI/HUDWidget.h"
#include "UI/Inventory/MainInventory.h"
#include "Blueprint/UserWidget.h"

#include "System/MapInfoSubsystem.h"

#include "Kismet/GameplayStatics.h"

ADungeonGameMode::ADungeonGameMode()
{
	// 기본 클래스 설정
	DefaultPawnClass = ADiaCharacter::StaticClass();
	
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

	if (IsValid(MapInfo))
	{
		MapInfo->CreateMapSpawnData();
	}
}

void ADungeonGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ADungeonGameMode::SpawnItemAtLocation(AActor* SpawnActor, const FItemBase& ItemData)
{
	if (IsValid(SpawnActor))
	{
		// 아이템 스폰
		FVector SpawnLocation = SpawnActor->GetActorLocation();
		SpawnLocation.Z += 100.f;

		ADiaItem* SpawnedItem = GetWorld()->SpawnActorDeferred<ADiaItem>(ADiaItem::StaticClass(), 
			 SpawnActor->GetTransform(), SpawnActor);
		
		UE_LOG(LogTemp, Warning, TEXT("Spawned Item at Location: %s"), *SpawnLocation.ToString());

		if (SpawnedItem)
		{
			SpawnedItem->SetItemProperty(ItemData);
			SpawnedItem->DropItem(ItemData);
			SpawnedItem->FinishSpawning(SpawnActor->GetTransform());
		}
	}
}

UHUDWidget* const ADungeonGameMode::GetHUDWidget() const
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	ADiaController* DiaPC = Cast<ADiaController>(PC);
	if (!DiaPC)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADungeonGameMode::GetHUDWidget - DiaController is null"));
		return nullptr;
	}

	// 실제 HUD는 컨트롤러가 관리
	return DiaPC->GetHUDWidget();

}