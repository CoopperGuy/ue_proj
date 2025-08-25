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

}

void UDiaCustomGameViewPort::OnDragEnd_CreateCautionWidget()
{
	if (!bIsDraggingItem)
	{
		OnDragEnded();
	}
	else
	{
		UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(CurrentDragOperation);

		CreateCautionWidget(ItemDragOp);
	}

	bIsDraggingItem = false;
	CurrentDragOperation = nullptr;
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

void UDiaCustomGameViewPort::CreateCautionWidget(UItemDragDropOperation* DragOp)
{
	// 이미 CautionWidget이 활성화되어 있는지 확인
	if (IsCautionWidgetAlreadyActive())
	{
		return;
	}

	// 인벤토리 컴포넌트 가져오기
	UDiaInventoryComponent* InvenComp = GetInventoryComponent();
	if (!IsValid(InvenComp))
	{
		return;
	}

	// CautionWidget 생성 및 초기화
	UDiaCaution* CautionWidget = CreateAndInitializeCautionWidget(DragOp);
	if (!IsValid(CautionWidget))
	{
		return;
	}

	// 아이템 데이터 가져오기
	UItemDragDropOperation* ItemDragOp = Cast<UItemDragDropOperation>(DragOp);
	if (!IsValid(ItemDragOp))
	{
		return;
	}
	FInventorySlot ItemData = ItemDragOp->ItemData;

	// 이벤트 바인딩
	BindCautionWidgetEvents(CautionWidget, ItemData, InvenComp);

	// 위젯 표시
	DisplayCautionWidget(CautionWidget);
}

bool UDiaCustomGameViewPort::IsCautionWidgetAlreadyActive() const
{
	if (!IsValid(DiaPrimaryLayout))
	{
		return false;
	}

	UCommonActivatableWidgetStack* HudLayer = DiaPrimaryLayout->GetLayerByTag(DiaPrimaryLayout->DefaultHudTag);
	if (!IsValid(HudLayer))
	{
		return false;
	}

	UCommonActivatableWidget* ActiveWidget = HudLayer->GetActiveWidget();
	return IsValid(ActiveWidget) && IsValid(Cast<UDiaCaution>(ActiveWidget));
}

UDiaCaution* UDiaCustomGameViewPort::CreateAndInitializeCautionWidget(UItemDragDropOperation* DragOp)
{
	// 위젯 클래스 로드
	UClass* WidgetClass = LoadClass<UDiaCaution>(nullptr, TEXT("/Game/UI/Caution/WBP_Caution.WBP_Caution_C"));
	if (!IsValid(WidgetClass))
	{
		return nullptr;
	}

	// 위젯 생성
	UDiaCaution* CautionWidget = CreateWidget<UDiaCaution>(GameInstance, WidgetClass);
	if (!IsValid(CautionWidget))
	{
		return nullptr;
	}

	// 위젯 초기화
	CautionWidget->SetVisibility(ESlateVisibility::Visible);
	CautionWidget->SetCautionText(FText::FromString("Are you sure you want to proceed?"));

	return CautionWidget;
}

UDiaInventoryComponent* UDiaCustomGameViewPort::GetInventoryComponent() const
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		return nullptr;
	}

	ADiaController* PlayerControllerRef = Cast<ADiaController>(PC);
	if (!IsValid(PlayerControllerRef))
	{
		return nullptr;
	}

	UDiaInventoryComponent* InvenComp = Cast<UDiaInventoryComponent>(
		PlayerControllerRef->GetComponentByClass(UDiaInventoryComponent::StaticClass()));

	return InvenComp;
}

void UDiaCustomGameViewPort::BindCautionWidgetEvents(UDiaCaution* CautionWidget, const FInventorySlot& ItemData, UDiaInventoryComponent* InvenComp)
{
	if (!IsValid(CautionWidget) || !IsValid(InvenComp))
	{
		return;
	}

	// 확인 버튼 바인딩
	CautionWidget->BindOnConfirmClicked(FOnConfirmClicked::CreateLambda([this, CautionWidget, ItemData, InvenComp]() {
		CautionWidget->DeactivateWidget();
		CautionWidget->RemoveFromParent();

		if (IsValid(DiaPrimaryLayout))
		{
			DiaPrimaryLayout->PopTopFromLayer(DiaPrimaryLayout->DefaultHudTag);
		}

		InvenComp->OnItemRemoved.Broadcast(ItemData.ItemInstance.InstanceID);
	}));

	// 취소 버튼 바인딩
	CautionWidget->BindOnCancelClicked(FOnCancelClicked::CreateLambda([this, CautionWidget]() {
		CautionWidget->DeactivateWidget();
		if (IsValid(DiaPrimaryLayout))
		{
			DiaPrimaryLayout->PopTopFromLayer(DiaPrimaryLayout->DefaultHudTag);
		}

		CautionWidget->RemoveFromParent();
		CautionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}));
}

void UDiaCustomGameViewPort::DisplayCautionWidget(UDiaCaution* CautionWidget)
{
	if (!IsValid(CautionWidget))
	{
		return;
	}

	CautionWidget->AddToViewport(999);

	if (IsValid(DiaPrimaryLayout))
	{
		DiaPrimaryLayout->PushToHudLayer(DiaPrimaryLayout->DefaultHudTag, CautionWidget);
	}
}
