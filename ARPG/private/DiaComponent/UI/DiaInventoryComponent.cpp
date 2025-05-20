// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "GameMode/DungeonGameMode.h"
#include "UI/HUDWidget.h"
#include "UI/Item/ItemWidget.h"
#include "UI/Inventory/MainInventory.h"

UDiaInventoryComponent::UDiaInventoryComponent()
	:InventoryGrid(GridWidth, GridHeight)
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UDiaInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void UDiaInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

bool UDiaInventoryComponent::TryAddItem(const FInventoryItem& ItemData)
{
	ADungeonGameMode* DungeonGameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(DungeonGameMode)) return false;
	UHUDWidget* HUD = DungeonGameMode->GetHUDWidget();
	if (!IsValid(HUD)) return false;
	UMainInventory* Inven = HUD->GetInventoryWidget();
	if (!Inven) return false;

	constexpr int32 InvenIdx = 0;

	UItemWidget* ItemWidget = Cast<UItemWidget>(Inven->GetItemWidgetAt(InvenIdx));
	//해당 index의 위젯이 존재해야한다
	//해당 칸이 비어있어야 한다.
	if (IsValid(ItemWidget) && !InventoryGrid[InvenIdx])
	{
		ItemWidget->SetItemInfo(ItemData);
		//ui작업
		bool res = Inven->AddItemToInventory(InvenIdx);
		//삽입에 성공하면, 여기서도 아이템이 삽입 되었다는 사실을 추가한다.
		if (res)
		{
			Items.Add(ItemData);
			InventoryGrid[InvenIdx] = true;
		}
	}
	
	
	return false;
}

bool UDiaInventoryComponent::CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
	return false;
}

