// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DungeonGameMode.h"
#include "Character/DiaCharacter.h"

#include "Item/DiaItem.h"

#include "UI/HUDWidget.h"
#include "Blueprint/UserWidget.h"

#include "System/MapInfoSubsystem.h"

#include "Kismet/GameplayStatics.h"

ADungeonGameMode::ADungeonGameMode()
{
	// 기본 클래스 설정
	DefaultPawnClass = ADiaCharacter::StaticClass();
	
	// 기본 HUD 위젯 클래스 설정 (블루프린트에서 오버라이드 가능)
	static ConstructorHelpers::FClassFinder<UHUDWidget> HUDWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_HUDWidget.WBP_HUDWidget"));
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

	// HUD 위젯 생성 및 표시
	if (HUDWidgetClass)
	{
		APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(playerController))
		{
			HUDWidgetInstance = CreateWidget<UHUDWidget>(playerController, HUDWidgetClass);            
			if (IsValid(HUDWidgetInstance))
			{
				HUDWidgetInstance->AddToViewport();
				
				// 인벤토리 위젯을 초기에 숨김 상태로 설정
				if (UMainInventory* InventoryWidget = HUDWidgetInstance->GetInventoryWidget())
				{
					InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
				}
			}
		}
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
		ADiaItem* SpawnedItem = GetWorld()->SpawnActor<ADiaItem>(ADiaItem::StaticClass(), 
			SpawnActor->GetActorLocation(), FRotator::ZeroRotator);
		if (SpawnedItem)
		{
			SpawnedItem->SetItemProperty(ItemData);
			SpawnedItem->DropItem(ItemData);
		}
	}
}
