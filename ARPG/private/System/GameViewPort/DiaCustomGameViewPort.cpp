// Fill out your copyright notice in the Description page of Project Settings.


#include "System/GameViewPort/DiaCustomGameViewPort.h"
#include "Engine/Engine.h"
#include "UI/DragDrop/ItemDragDropOperation.h"
#include "UI/Item/ItemWidget.h"

void UDiaCustomGameViewPort::Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice)
{
	Super::Init(WorldContext, OwningGameInstance, bCreateNewAudioDevice);
	
	// 필요한 초기화 로직만 여기에 추가
	UE_LOG(LogTemp, Warning, TEXT("DiaCustomGameViewPort initialized successfully"));
}

void UDiaCustomGameViewPort::Draw(FViewport* InViewport, FCanvas* SceneCanvas)
{
	Super::Draw(InViewport, SceneCanvas);

	UE_LOG(LogTemp, Warning, TEXT("DiaCustomGameViewPort Draw"));
	 
}

void UDiaCustomGameViewPort::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UE_LOG(LogTemp, Warning, TEXT("DiaCustomGameViewPort Tick"));

}

void UDiaCustomGameViewPort::LostFocus(FViewport* InViewport)
{
	Super::LostFocus(InViewport);
}

bool UDiaCustomGameViewPort::InputKey(const FInputKeyEventArgs& EventArgs)
{
	// 마우스 왼쪽 버튼 릴리즈 감지
	if (EventArgs.Key == EKeys::LeftMouseButton && EventArgs.Event == IE_Released)
	{
		if (bIsDraggingItem && CurrentDragOperation)
		{
			FIntPoint MousePosition;
			EventArgs.Viewport->GetMousePos(MousePosition);  // InViewport 사용

			HandleDropInVoid(MousePosition);
			OnDragEnded();

			UE_LOG(LogTemp, Warning, TEXT("Item dropped in void at position: %d, %d"), MousePosition.X, MousePosition.Y);
			return true;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("DiaCustomGameViewPort InputKey"));

	return Super::InputKey(EventArgs);
}

void UDiaCustomGameViewPort::OnDragStarted(UItemDragDropOperation* DragOp)
{
	bIsDraggingItem = true;
	CurrentDragOperation = DragOp;
	UE_LOG(LogTemp, Warning, TEXT("Drag started - monitoring for void drop"));
}

void UDiaCustomGameViewPort::OnDragEnded()
{
	bIsDraggingItem = false;
	CurrentDragOperation = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("OnDragEnded - monitoring for void drop"));
}

void UDiaCustomGameViewPort::HandleDropInVoid(const FIntPoint& MousePosition)
{
	if (CurrentDragOperation && CurrentDragOperation->SourceWidget)
	{
		// 아이템을 원래 위치로 되돌리기
		CurrentDragOperation->SourceWidget->SetRenderOpacity(1.0f);
		
		UE_LOG(LogTemp, Warning, TEXT("Item %s dropped in void - restoring original position"), 
			*CurrentDragOperation->ItemData.ItemInstance.BaseItem.Name.ToString());
		
		// 추가적인 처리 (예: 특별한 이펙트, UI 피드백 등)		// 여기에 원하는 허공 드롭 로직 추가

	}
}
