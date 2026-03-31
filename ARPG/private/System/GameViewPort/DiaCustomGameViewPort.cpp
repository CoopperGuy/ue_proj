// Fill out your copyright notice in the Description page of Project Settings.


#include "System/GameViewPort/DiaCustomGameViewPort.h"
#include "GameFramework/InputSettings.h"

#include "Controller/DiaController.h"
#include "DiaComponent/UI/DiaInventoryComponent.h"

#include "Widgets/CommonActivatableWidgetContainer.h"

#include "Engine/Engine.h"

#include "UI/DragDrop/ItemDragDropOperation.h"
#include "UI/System/DiaPrimaryLayout.h"
#include "UI/DiaCaution.h"
#include "UI/Item/ItemWidget.h"

void UDiaCustomGameViewPort::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);

	UClass* WidgetClass = LoadClass<UDiaPrimaryLayout>(nullptr, TEXT("/Game/UI/System/WBP_DiaPrimaryLayOut.WBP_DiaPrimaryLayOut_C"));
	if (WidgetClass)
	{
		DiaPrimaryLayout = CreateWidget<UDiaPrimaryLayout>(OwningGameInstance, WidgetClass);
		DiaPrimaryLayout->InitLayers();
	}

}

void UDiaCustomGameViewPort::Draw(FViewport* InViewport, FCanvas* SceneCanvas)
{
	Super::Draw(InViewport, SceneCanvas);
}

void UDiaCustomGameViewPort::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void UDiaCustomGameViewPort::LostFocus(FViewport* InViewport)
{
	Super::LostFocus(InViewport);
}

bool UDiaCustomGameViewPort::InputKey(const FInputKeyEventArgs& EventArgs)
{
	return Super::InputKey(EventArgs);
}

// void UDiaCustomGameViewPort::OnDragStarted(UItemDragDropOperation* DragOp)
// {
// 	bIsDraggingItem = true;
// 	CurrentDragOperation = DragOp;
// }

// void UDiaCustomGameViewPort::OnDragEnded()
// {
// 	bIsDraggingItem = false;
// 	CurrentDragOperation = nullptr;
// }

// void UDiaCustomGameViewPort::OnDragEnd_CreateCautionWidget()
// {
// 	if (!bIsDraggingItem)
// 	{
// 		OnDragEnded();
// 	}
// 	else
// 	{
// 		UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(CurrentDragOperation);
// 		CreateCautionWidget(ItemDragOp);
// 	}
// 	bIsDraggingItem = false;
// 	CurrentDragOperation = nullptr;
// }

// void UDiaCustomGameViewPort::HandleDropInVoid(const FIntPoint& MousePosition)
// {
// 	if (CurrentDragOperation && CurrentDragOperation->SourceWidget)
// 	{
// 		CurrentDragOperation->SourceWidget->SetRenderOpacity(1.0f);
// 	}
// }

// void UDiaCustomGameViewPort::CreateCautionWidget(UItemDragDropOperation* DragOp)
// {
// 	if (IsCautionWidgetAlreadyActive()) return;
// 	UDiaInventoryComponent* InvenComp = GetInventoryComponent();
// 	if (!IsValid(InvenComp)) return;
// 	UDiaCaution* CautionWidget = CreateAndInitializeCautionWidget(DragOp);
// 	if (!IsValid(CautionWidget)) return;
// 	UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(DragOp);
// 	if (!IsValid(ItemDragOp)) return;
// 	FInventorySlot ItemData = ItemDragOp->ItemData;
// 	BindCautionWidgetEvents(CautionWidget, ItemData, InvenComp);
// 	DisplayCautionWidget(CautionWidget);
// }

void UDiaCustomGameViewPort::SetupGameInstance(UDiaInstance* DiaInstance)
{
	if(!IsValid(DiaPrimaryLayout))
	{
		return;
	}

	DiaPrimaryLayout->AddToViewport();
	DiaPrimaryLayout->SetVisibility(ESlateVisibility::Visible);
}

// bool UDiaCustomGameViewPort::IsCautionWidgetAlreadyActive() const { ... }
// UDiaCaution* UDiaCustomGameViewPort::CreateAndInitializeCautionWidget(...) { ... }
// UDiaInventoryComponent* UDiaCustomGameViewPort::GetInventoryComponent() const { ... }
// void UDiaCustomGameViewPort::BindCautionWidgetEvents(...) { ... }
// void UDiaCustomGameViewPort::DisplayCautionWidget(...) { ... }
