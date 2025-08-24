// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/System/DiaPrimaryLayout.h"
#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UDiaPrimaryLayout::NativeConstruct()
{
	Super::NativeConstruct();
}

UCommonActivatableWidgetStack* UDiaPrimaryLayout::GetLayerByTag(const FGameplayTag InTag) const
{
	const auto FounWidget = HudWidgets.Find(InTag);
	if (FounWidget)
	{
		return *FounWidget;
	}
	return nullptr;
}

UUserWidget* UDiaPrimaryLayout::PushToHudLayer(const FGameplayTag InTag, UCommonActivatableWidget* Widget)
{
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("PushToHudLayer: Widget이 nullptr입니다."));
		return nullptr;
	}
	
	UCommonActivatableWidgetStack* Layer = GetLayerByTag(InTag);
	if (!Layer)
	{
		UE_LOG(LogTemp, Warning, TEXT("No layer found for tag: %s"), *InTag.ToString());
		return nullptr;
	}
	
	// 이미 이 위젯이 레이어에 있는지 확인
	if (Layer->GetActiveWidget() == Widget)
	{
		return Widget;
	}
	
	// DiaPrimaryLayout이 Viewport에 있는지 확인
	if (!this->IsInViewport())
	{
		this->AddToViewport();
	}
	
	// 레이어 가시성 확보
	Layer->SetVisibility(ESlateVisibility::Visible);
	
	// 위젯 가시성 설정
	Widget->SetVisibility(ESlateVisibility::Visible);
	
	// 먼저 위젯을 부모에서 제거 (이미 다른 부모에 있을 경우)
	if (Widget->GetParent())
	{
		Widget->RemoveFromParent();
	}
	
	// Stack에 추가
	Layer->AddWidgetInstance(*Widget);
	
	// CommonActivatableWidget 명시적 활성화
	if (UCommonActivatableWidget* ActivatableWidget = Cast<UCommonActivatableWidget>(Widget))
	{
		ActivatableWidget->ActivateWidget();
	}

	return Widget;
}

void UDiaPrimaryLayout::PopTopFromLayer(const FGameplayTag InTag)
{
	UCommonActivatableWidgetStack* Layer = GetLayerByTag(InTag);
	if (!Layer)
	{
		UE_LOG(LogTemp, Warning, TEXT("No layer found for tag: %s"), *InTag.ToString());
		return;
	}
	
	Layer->SetVisibility(ESlateVisibility::Collapsed);
	// 스택에서 최상단 위젯 가져와서 제거
	if (UCommonActivatableWidget* TopWidget = Layer->GetActiveWidget())
	{
		TopWidget->SetVisibility(ESlateVisibility::Collapsed);
		Layer->RemoveWidget(*TopWidget);
	}
}

void UDiaPrimaryLayout::InitLayers()
{
	DefaultHudTag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.HUD"));
	DefaultModelTag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Model"));
	
	if (HudLayer)
	{
		HudLayer->SetVisibility(ESlateVisibility::Visible);
		HudLayer->SetRenderOpacity(1.0f);
		HudWidgets.Add(DefaultHudTag, HudLayer);
	}
	
	if (ModelLayer)
	{
		ModelLayer->SetVisibility(ESlateVisibility::Visible);
		ModelLayer->SetRenderOpacity(1.0f);
		HudWidgets.Add(DefaultModelTag, ModelLayer);
	}
}
