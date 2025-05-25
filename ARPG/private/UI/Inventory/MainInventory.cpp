// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/MainInventory.h"
#include "UI/Item/ItemWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"

#include "Blueprint/WidgetTree.h"

void UMainInventory::NativeConstruct()
{
	Super::NativeConstruct();

	CreateInventory();
}

bool UMainInventory::IsSlotEmpty(int32 SlotIndex) const
{
	if (!InventorySlots.IsValidIndex(SlotIndex))
		return false;
	
	UItemWidget* ItemWidget = GetItemWidgetAt(SlotIndex);
	if (!ItemWidget)
		return true;
	
	// ItemWidget이 있지만 아이템 정보가 없거나 숨겨진 경우
	return ItemWidget->GetVisibility() != ESlateVisibility::Visible;
}

bool UMainInventory::AddItemToInventory(const FInventoryItem& ItemData, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
	// 범위 체크 추가
	if (PosX < 0 || PosY < 0 || PosX + ItemWidth > 10 || PosY + ItemHeight > 10)
		return false;
	
	// 첫 번째 슬롯에만 아이템 위젯 생성하고 크기 조정
	int32 MainSlotIndex = PosY * 10 + PosX;
	UItemWidget* MainItemWidget = GetItemWidgetAt(MainSlotIndex);
	
	if (MainItemWidget)
	{
		MainItemWidget->SetItemInfo(ItemData);
		MainItemWidget->SetVisibility(ESlateVisibility::Visible);
		

		//해당 로직은 아이템이 차지하는 공간을 숨기는 로직이다.
		//임시 로직 -> 적용 고민중.
		// 아이템 크기에 맞게 위젯 크기 조정
		//UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MainItemWidget->Slot);
		//if (CanvasSlot)
		//{
		//	FVector2D NewSize = FVector2D(szSlot * ItemWidth, szSlot * ItemHeight);
		//	CanvasSlot->SetSize(NewSize);
		//}
		//
		//// 나머지 슬롯들은 숨김 처리 (아이템이 차지하는 공간)
		//for (int32 y = PosY; y < PosY + ItemHeight; ++y)
		//{
		//	for (int32 x = PosX; x < PosX + ItemWidth; ++x)
		//	{
		//		if (x == PosX && y == PosY) continue; // 메인 슬롯은 제외
		//		
		//		int32 SlotIndex = y * 10 + x;
		//		UItemWidget* SlotWidget = GetItemWidgetAt(SlotIndex);
		//		if (SlotWidget)
		//		{
		//			SlotWidget->SetVisibility(ESlateVisibility::Hidden);
		//		}
		//	}
		//}
		
		return true;
	}
	
	return false;
}

void UMainInventory::CreateInventory()
{
	constexpr int32 szInventory = 10;
	
	// 위젯 클래스 한 번만 로드
	FSoftObjectPath ItemWidgetPath(TEXT("/Game/UI/Inventory/WBP_ItemWidget.WBP_ItemWidget_C"));
	TSoftClassPtr<UItemWidget> WidgetAssetPtr(ItemWidgetPath);
	UClass* ItemWidgetClass = WidgetAssetPtr.LoadSynchronous();

	if (!ItemWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ItemWidget class"));
		return;
	}
	
	for (int32 y = 0; y < szInventory; ++y)
	{
		for (int32 x = 0; x < szInventory; ++x)
		{
			UItemWidget* ItemWidget = WidgetTree->ConstructWidget<UItemWidget>(ItemWidgetClass);
			
			if (ItemWidget)
			{
				UCanvasPanelSlot* NewSlot = InventoryCanvas->AddChildToCanvas(ItemWidget);
				int32 SlotIndex = y * szInventory + x;
				ConfigInventorySlot(SlotIndex, NewSlot);
				InventorySlots.Add(NewSlot);

				//초기에는 일단 숨긴다 (나중에 상호작요을 통해 보여줄 예정)
				//ItemWidget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void UMainInventory::ConfigInventorySlot(int32 SlotIndex, UCanvasPanelSlot* CanvasSlot)
{
	if (CanvasSlot)
	{
		FVector2D Position = FVector2D((SlotIndex % 10) * szSlot, (SlotIndex / 10) * szSlot);
		FVector2D Size = FVector2D(szSlot, szSlot);
		CanvasSlot->SetPosition(Position);
		CanvasSlot->SetSize(Size);
	}
}

FORCEINLINE UItemWidget* UMainInventory::GetItemWidgetAt(int32 Index) const
{
	if (!InventorySlots.IsValidIndex(Index))
	{
		return nullptr;
	}
	return Cast<UItemWidget>(InventorySlots[Index]->GetContent());
}

void UMainInventory::GetAllItemWidgets(TArray<UItemWidget*>& OutItemWidgets) const
{
	OutItemWidgets.Empty();
	
	for (UCanvasPanelSlot* ItemSlot : InventorySlots)
	{
		UItemWidget* ItemWidget = Cast<UItemWidget>(ItemSlot->GetContent());
		if (ItemWidget)
		{
			OutItemWidgets.Add(ItemWidget);
		}
	}	
}