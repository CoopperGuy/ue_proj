// Fill out your copyright notice in the Description page of Project Settings.


#include "System/GameViewPort/DiaCustomGameViewPort.h"
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
		//생성 관련 로그 추가
		UE_LOG(LogTemp, Warning, TEXT("UDiaPrimaryLayout 생성 성공: %s"), *DiaPrimaryLayout->GetClass()->GetName());
	}

	DiaPrimaryLayout->AddToViewport();
	DiaPrimaryLayout->SetVisibility(ESlateVisibility::Visible);
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

void UDiaCustomGameViewPort::OnDragStarted(UItemDragDropOperation* DragOp)
{
	bIsDraggingItem = true;
	CurrentDragOperation = DragOp;
}

void UDiaCustomGameViewPort::OnDragEnded()
{
	bIsDraggingItem = false;
	CurrentDragOperation = nullptr;

	CreateCautionWidget();
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

void UDiaCustomGameViewPort::CreateCautionWidget()
{
	// 이미 존재하는 CautionWidget이 있는지 확인
	UCommonActivatableWidgetStack* HudLayer = DiaPrimaryLayout->GetLayerByTag(DiaPrimaryLayout->DefaultHudTag);
	if (HudLayer)
	{
		// 이미 활성화된 위젯이 있는지 확인
		if (UCommonActivatableWidget* ActiveWidget = HudLayer->GetActiveWidget())
		{
			// UDiaCaution 타입인지 확인
			if (Cast<UDiaCaution>(ActiveWidget))
			{
				return; // 이미 존재하므로 새로 생성하지 않음
			}
		}
	}

	// 새로운 CautionWidget 생성
	UClass* WidgetClass = LoadClass<UDiaCaution>(nullptr, TEXT("/Game/UI/Caution/WBP_Caution.WBP_Caution_C"));
	if (!IsValid(WidgetClass))
	{
		return;
	}

	// GameInstance를 사용하여 위젯 생성
	UDiaCaution* CatuionWidget = CreateWidget<UDiaCaution>(GameInstance, WidgetClass);
	if (IsValid(CatuionWidget))
	{
		// 위젯을 명시적으로 표시
		CatuionWidget->SetVisibility(ESlateVisibility::Visible);
		
		CatuionWidget->SetCautionText(FText::FromString("Are you sure you want to proceed?"));

		// 확인, 캔슬 버튼 바인딩
		CatuionWidget->BindOnConfirmClicked(FOnConfirmClicked::CreateLambda([CatuionWidget]() {
			CatuionWidget->RemoveFromParent();
		}));

		CatuionWidget->BindOnCancelClicked(FOnCancelClicked::CreateLambda([CatuionWidget]() {
			CatuionWidget->RemoveFromParent();
		}));

		// 직접 AddToViewport 사용
		CatuionWidget->AddToViewport(999); // 높은 Z-Order
		
		// Layer 방식이 필요한 경우
		if (!CatuionWidget->IsInViewport())
		{
			DiaPrimaryLayout->PushToHudLayer(DiaPrimaryLayout->DefaultHudTag, CatuionWidget);
		}
	}
}
