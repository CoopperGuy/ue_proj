// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Menu/DiaMainMenuWorkspace.h"

#include "CommonActivatableWidgetSwitcher.h"
#include "CommonInputModeTypes.h"
#include "Blueprint/WidgetTree.h"
#include "Logging/ARPGLogChannels.h"

UDiaMainMenuWorkspace::UDiaMainMenuWorkspace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 별도의 Back Input Action이 설정되어 있지 않아 매번 잘못된 액션 바인딩 오류가 발생했다.
	// 메뉴 토글은 기존 Enhanced Input 경로에서 처리한다.
	bIsBackHandler = false;
}

void UDiaMainMenuWorkspace::NativeConstruct()
{
	Super::NativeConstruct();

	// 전체 화면 메뉴 컨테이너는 빈 영역에서 클릭을 소비하지 않고,
	// 실제 자식 위젯(버튼, 리스트, 아이템 슬롯)만 히트 테스트에 참여한다.
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (WidgetTree && IsValid(WidgetTree->RootWidget))
	{
		WidgetTree->RootWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (NavigationStack)
	{
		NavigationStack->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (ContentStack)
	{
		ContentStack->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

TOptional<FUIInputConfig> UDiaMainMenuWorkspace::GetDesiredInputConfig() const
{
	return FUIInputConfig(ECommonInputMode::All, EMouseCaptureMode::NoCapture);
}

UCommonActivatableWidget* UDiaMainMenuWorkspace::GetActiveNavigation() const
{
	return NavigationStack ? Cast<UCommonActivatableWidget>(NavigationStack->GetActiveWidget()) : nullptr;
}

UCommonActivatableWidget* UDiaMainMenuWorkspace::ShowPage(const FGameplayTag& InTag)
{
	const FDiaMenuPageDefinition* PageDefinition = MenuPages.Find(InTag);
	if (!PageDefinition || !PageDefinition->WidgetClass)
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaMainMenuWorkspace::ShowPage - No page definition or widget class for tag: %s"),
			*InTag.ToString());
		return nullptr;
	}

	UCommonActivatableWidgetSwitcher* Switcher = nullptr;
	FGameplayTag* ActivePageTag = nullptr;

	switch (PageDefinition->Region)
	{
	case EDiaMenuPageRegion::Navigation:
		Switcher = NavigationStack;
		ActivePageTag = &ActiveNavigationTag;
		break;
	case EDiaMenuPageRegion::Content:
		Switcher = ContentStack;
		ActivePageTag = &ActiveContentTag;
		break;
	default:
		return nullptr;
	}

	if (!Switcher || !ActivePageTag)
	{
		return nullptr;
	}

	UCommonActivatableWidget* PageWidget = ActiveMenuPages.FindRef(InTag);
	if (!IsValid(PageWidget))
	{
		PageWidget = CreateWidget<UCommonActivatableWidget>(GetOwningPlayer(), PageDefinition->WidgetClass);
		if (!IsValid(PageWidget) || !Switcher->AddChild(PageWidget))
		{
			UE_LOG(LogARPG, Warning, TEXT("DiaMainMenuWorkspace::ShowPage - Failed to create or add page widget: %s"),
				*GetNameSafe(PageDefinition->WidgetClass));
			return nullptr;
		}

		ActiveMenuPages.Add(InTag, PageWidget);
	}
	else if (!Switcher->HasChild(PageWidget))
	{
		if (!Switcher->AddChild(PageWidget))
		{
			return nullptr;
		}
	}

	// 활성 페이지 자체는 포커스를 받을 수 있게 하되, 페이지의 빈 배경은 클릭을 소비하지 않는다.
	// CommonUI가 포커스 대상을 찾지 못해 게임 뷰포트로 포커스를 되돌리는 것도 방지한다.
	PageWidget->SetIsFocusable(true);
	Switcher->SetActiveWidget(PageWidget);
	PageWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (PageWidget->WidgetTree && IsValid(PageWidget->WidgetTree->RootWidget))
	{
		PageWidget->WidgetTree->RootWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	*ActivePageTag = InTag;
	return PageWidget;
}

UCommonActivatableWidget* UDiaMainMenuWorkspace::GetActiveContent() const
{
	return ContentStack ? Cast<UCommonActivatableWidget>(ContentStack->GetActiveWidget()) : nullptr;
}

bool UDiaMainMenuWorkspace::IsPageActive(const FGameplayTag& InTag) const
{
	const FDiaMenuPageDefinition* PageDefinition = MenuPages.Find(InTag);
	if (!PageDefinition)
	{
		return false;
	}

	const UCommonActivatableWidget* PageWidget = ActiveMenuPages.FindRef(InTag);
	switch (PageDefinition->Region)
	{
	case EDiaMenuPageRegion::Navigation:
		return ActiveNavigationTag == InTag && GetActiveNavigation() == PageWidget;
	case EDiaMenuPageRegion::Content:
		return ActiveContentTag == InTag && GetActiveContent() == PageWidget;
	default:
		return false;
	}
}
