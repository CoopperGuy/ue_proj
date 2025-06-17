// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/DiaController.h"
#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "UI/HUDWidget.h"
#include "UI/Inventory/MainInventory.h"
#include "GameMode/DungeonGameMode.h"

ADiaController::ADiaController()
{
	bShowMouseCursor = true;
	DiaInventoryComponent = CreateDefaultSubobject<UDiaInventoryComponent>(TEXT("InventoryComponent"));

}

void ADiaController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(DiaInventoryComponent))
	{
		DiaInventoryComponent->RegisterComponent();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaInventoryComponent is null"));
	}
}

void ADiaController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

UHUDWidget* ADiaController::GetHUDWidget() const
{
	// 캐시된 HUDWidget이 유효하면 그대로 반환
	if (CachedHUDWidget.IsValid())
	{
		return CachedHUDWidget.Get();
	}

	// 캐시되지 않았거나 무효하면 GameMode에서 가져와서 캐시
	ADungeonGameMode* GameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(GameMode))
	{
		UHUDWidget* HUDWidget = GameMode->GetHUDWidget();
		if (IsValid(HUDWidget))
		{
			CachedHUDWidget = HUDWidget;
			return HUDWidget;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HUDWidget from GameMode is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonGameMode is null"));
	}

	return nullptr;
}

//인벤토리에 아이템 더하기 호출.
//외부에서 호출하자.
bool ADiaController::ItemAddedToInventory(const FInventoryItem& Item)
{
	if (!IsValid(DiaInventoryComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("DiaInventoryComponent is null"));
		return false;
	}

	ADungeonGameMode* GameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonGameMode is null"));
		return false;
	}

	UHUDWidget* HUDWidget = GetHUDWidget();
	//인벤토리 위젯을 가져와서 넣는다.
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return false;
	}

	//검증은 tryadditem에서 한다.
	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();

	bool bResult = DiaInventoryComponent->TryAddItem(Item, InventoryWidget);
	if (bResult)
	{
		UE_LOG(LogTemp, Log, TEXT("Item successfully added to inventory: %s"), *Item.ItemID.ToString());
	}

	return bResult;
}

void ADiaController::ItemRemoved(const FInventoryItem& Item)
{
	if (!IsValid(DiaInventoryComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("DiaInventoryComponent is null"));
		return;
	}

	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return;
	}

	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();
	if (!InventoryWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget is null"));
		return;
	}

	bool bResult = DiaInventoryComponent->RemoveItem(Item.InstanceID, InventoryWidget);
	if (bResult)
	{
		UE_LOG(LogTemp, Log, TEXT("Item successfully removed from inventory: %s"), *Item.ItemID.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to remove item from inventory: %s"), *Item.ItemID.ToString());
	}
}

void ADiaController::ToggleInventoryVisibility(bool bVisible)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return;
	}

	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();
	if (!InventoryWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget is null"));
		return;
	}

	InventoryWidget->SetVisibility((bVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

ESlateVisibility ADiaController::GetInventoryVisibility() const
{
	const UHUDWidget* const HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return ESlateVisibility::Collapsed;
	}

	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();
	if (!InventoryWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget is null"));
		return ESlateVisibility::Collapsed;
	}

	return InventoryWidget->GetVisibility();
}
