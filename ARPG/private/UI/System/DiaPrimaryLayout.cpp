// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/System/DiaPrimaryLayout.h"
#include "UI/HUDWidget.h"
#include "UI/Menu/DiaMainMenuWorkspace.h"


#include "CommonActivatableWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Blueprint/WidgetTree.h"

void UDiaPrimaryLayout::NativeConstruct()
{
	Super::NativeConstruct();

	// Blueprint 디자이너에서 최상위 Root가 HitTestInvisible이면 이 UserWidget의
	// SelfHitTestInvisible 설정과 무관하게 모든 자식 UI가 Slate 히트 테스트에서 제외된다.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (WidgetTree && IsValid(WidgetTree->RootWidget))
	{
		WidgetTree->RootWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (DefaultHudWidgetClass && !GetHUDWidget())
	{
		HUDWidget = Cast<UHUDWidget>(PushToLayer(DefaultHudTag, DefaultHudWidgetClass));
	}

}

UCommonActivatableWidgetStack* UDiaPrimaryLayout::GetLayerByTag(const FGameplayTag & InTag) const
{
	UCommonActivatableWidgetStack* FounWidget = HudWidgets.FindRef(InTag);
	if (FounWidget)
	{
		return FounWidget;
	}

	return nullptr;
}

void UDiaPrimaryLayout::PopTopFromLayer(const FGameplayTag & InTag)
{
	UCommonActivatableWidgetStack* Layer = GetLayerByTag(InTag);
	if (!Layer)
	{
		return;
	}

	// 스택에서 최상단 위젯 가져와서 제거
	if (UCommonActivatableWidget* TopWidget = Layer->GetActiveWidget())
	{
		Layer->RemoveWidget(*TopWidget);
	}
}

UCommonActivatableWidget* UDiaPrimaryLayout::PushToLayer(const FGameplayTag & InTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!this->IsInViewport())
	{
		this->AddToViewport();
	}

	if (!WidgetClass)
	{
		return nullptr;
	}

	UCommonActivatableWidgetStack* Layer = GetLayerByTag(InTag);
	if (!Layer)
	{
		UE_LOG(LogARPG, Warning, TEXT("PushToLayer No layer found for tag: %s"), *InTag.ToString());
		return nullptr;
	}

	UCommonActivatableWidget* Widget = Layer->AddWidget(WidgetClass);
	if (!Widget)
	{
		UE_LOG(LogARPG, Warning, TEXT("PushToLayer Failed to create widget of class: %s"), *WidgetClass->GetName());
		return nullptr;
	}

	// UCommonActivatableWidgetStack가 활성화와 가시성을 관리한다.
	// 여기서 Visible을 강제하면 전체 화면 스택 자체가 마우스 히트 테스트 대상이 되어
	// 메뉴 안의 버튼이나 리스트보다 먼저 입력을 받을 수 있다.
	return Widget;
}

UDiaMainMenuWorkspace* UDiaPrimaryLayout::GetActiveMenuWorkspace() const
{
	return Cast<UDiaMainMenuWorkspace>(GetTopWidgetFromLayer(DefaultMenuTag));
}

UDiaMainMenuWorkspace* UDiaPrimaryLayout::GetOrCreateMenuWorkspace()
{
	if (UDiaMainMenuWorkspace* MenuWidget = GetActiveMenuWorkspace())
	{
		return MenuWidget;
	}

	if (!DefaultMenuWidgetClass)
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaPrimaryLayout::GetOrCreateMenuWorkspace - DefaultMenuWidgetClass is not set."));
		return nullptr;
	}

	return Cast<UDiaMainMenuWorkspace>(PushToLayer(DefaultMenuTag, DefaultMenuWidgetClass));
}

UCommonActivatableWidget* UDiaPrimaryLayout::OpenMenuPage(const FGameplayTag & UITag)
{
	UDiaMainMenuWorkspace* MenuWidget = GetOrCreateMenuWorkspace();
	if (!IsValid(MenuWidget))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaPrimaryLayout::OpenMenuPage - Failed to get or create the menu workspace."));
		return nullptr;
	}

	UCommonActivatableWidget* OpenedPage = MenuWidget->ShowPage(UITag);
	if (IsValid(OpenedPage))
	{
		// HUD는 항상 활성 상태이므로, Blueprint의 레이어 순서에 따라 메뉴보다 위에서
		// 전체 화면 마우스 히트 테스트를 가로챌 수 있다. 메뉴가 열린 동안 HUD 자식까지
		// 히트 테스트에서 제외하고 메뉴 레이어만 입력을 받게 한다.
		SetHUDInteractionEnabled(false);
		if (IsValid(MenuLayer))
		{
			MenuLayer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}

	return OpenedPage;
}

void UDiaPrimaryLayout::CloseMenu()
{
	UDiaMainMenuWorkspace* MenuWidget = GetActiveMenuWorkspace();
	UCommonActivatableWidgetStack* MenuWidgetLayer = GetLayerByTag(DefaultMenuTag);
	if (!IsValid(MenuWidget) || !IsValid(MenuWidgetLayer))
	{
		return;
	}

	MenuWidgetLayer->RemoveWidget(*MenuWidget);
	SetHUDInteractionEnabled(true);
}

void UDiaPrimaryLayout::SetHUDInteractionEnabled(bool bEnabled)
{
	if (IsValid(HudLayer))
	{
		HudLayer->SetVisibility(bEnabled
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::HitTestInvisible);
	}
}
UCommonActivatableWidget* UDiaPrimaryLayout::GetTopWidgetFromLayer(const FGameplayTag & InTag) const
{
	UCommonActivatableWidgetStack* Layer = GetLayerByTag(InTag);
	if (Layer)
	{
		return Layer->GetActiveWidget();
	}

	return nullptr;
}

void UDiaPrimaryLayout::InitLayers()
{
	DefaultHudTag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.HUD"));
	DefaultModelTag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Model"));
	DefaultMenuTag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Menu"));

	if (HudLayer)
	{
		//HudLayer->SetVisibility(ESlateVisibility::Visible);
		HudLayer->SetRenderOpacity(1.0f);
		HudWidgets.Add(DefaultHudTag, HudLayer);
	}
	
	if (ModalLayer)
	{
		//ModalLayer->SetVisibility(ESlateVisibility::Visible);
		ModalLayer->SetRenderOpacity(1.0f);
		HudWidgets.Add(DefaultModelTag, ModalLayer);
	}

	if(MenuLayer)
	{
		MenuLayer->SetRenderOpacity(1.0f);
		HudWidgets.Add(DefaultMenuTag, MenuLayer);
	}
}

void UDiaPrimaryLayout::DisableLayers()
{
	if (HudLayer)
	{
		HudLayer->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ModalLayer)
	{
		ModalLayer->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (MenuLayer)
	{
		MenuLayer->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool UDiaPrimaryLayout::IsMainMenuOpen() const
{
	return IsValid(GetActiveMenuWorkspace());
}

UHUDWidget* UDiaPrimaryLayout::GetHUDWidget() const
{
	return HUDWidget;
}

bool UDiaPrimaryLayout::ToggleMenu(const FGameplayTag& UITag)
{
	if (UDiaMainMenuWorkspace* MenuWidget = GetActiveMenuWorkspace())
	{
		if (MenuWidget->IsPageActive(UITag))
		{
			CloseMenu();
			return false;
		}
	}

	return IsValid(OpenMenuPage(UITag));
}
