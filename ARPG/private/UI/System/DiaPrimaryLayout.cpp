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

void UDiaPrimaryLayout::PopTopFromLayer(const FGameplayTag InTag)
{
	UCommonActivatableWidgetStack* Layer = GetLayerByTag(InTag);
	if (!Layer)
	{
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

void UDiaPrimaryLayout::DisableLayers()
{
	if (HudLayer)
	{
		HudLayer->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ModelLayer)
	{
		ModelLayer->SetVisibility(ESlateVisibility::Collapsed);
	}
}
