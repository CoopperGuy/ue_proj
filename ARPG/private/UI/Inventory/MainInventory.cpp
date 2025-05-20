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

bool UMainInventory::AddItemToInventory(int32 SlotIndex)
{
	if (InventorySlots.IsValidIndex(SlotIndex))
	{
		UCanvasPanelSlot* PanelSlot = InventorySlots[SlotIndex];
		if (PanelSlot)
		{
			return true;
		}
	}
	return false;
}

void UMainInventory::CreateInventory()
{
	constexpr int32 szInventory = 10;
	
	// 위젯 클래스 한 번만 로드
	FSoftObjectPath ItemWidgetPath(TEXT("/Game/UI/WorldWidget/WBP_ItemWidget.WBP_ItemWidget_C"));
	TSoftClassPtr<UItemWidget> WidgetAssetPtr(ItemWidgetPath);
	UClass* ItemWidgetClass = WidgetAssetPtr.LoadSynchronous();
	
	if (!ItemWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load ItemWidget class"));
		return;
	}
	
	for (int32 i = 0; i < szInventory; ++i)
	{
		for (int32 j = 0; j < szInventory; ++j)
		{
			UItemWidget* ItemWidget = WidgetTree->ConstructWidget<UItemWidget>(ItemWidgetClass);
			
			if (ItemWidget)
			{
				UCanvasPanelSlot* NewSlot = InventoryCanvas->AddChildToCanvas(ItemWidget);
				int32 SlotIndex = i * szInventory + j;
				ConfigInventorySlot(SlotIndex, NewSlot);
				InventorySlots.Add(NewSlot);
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
	return Cast<UItemWidget>(InventorySlots[Index]->GetContent());
}

void UMainInventory::GetAllItemWidgets(TArray<UItemWidget*>& OutItemWidgets) const
{
	OutItemWidgets.Empty();
	
	for (UCanvasPanelSlot* Slot : InventorySlots)
	{
		UItemWidget* ItemWidget = Cast<UItemWidget>(Slot->GetContent());
		if (ItemWidget)
		{
			OutItemWidgets.Add(ItemWidget);
		}
	}	
}