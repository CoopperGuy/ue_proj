#include "UI/Menu/DiaSessionMenuWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "CommonInputModeTypes.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Controller/DiaController.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "DiaSessionMenu"

UDiaSessionMenuWidget::UDiaSessionMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsModal = true;
    bIsBackHandler = false;
    SetIsFocusable(true);
}

TSharedRef<SWidget> UDiaSessionMenuWidget::RebuildWidget()
{
    if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
    if (!WidgetTree->RootWidget)
    {
        UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>();
        WidgetTree->RootWidget = Root;

        UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>();
        Backdrop->SetBrushColor(FLinearColor(.012f, .017f, .026f, .96f));
        UOverlaySlot* BackdropSlot = Root->AddChildToOverlay(Backdrop);
        BackdropSlot->SetHorizontalAlignment(HAlign_Fill);
        BackdropSlot->SetVerticalAlignment(VAlign_Fill);

        USizeBox* CardSize = WidgetTree->ConstructWidget<USizeBox>();
        CardSize->SetWidthOverride(440.f);
        UOverlaySlot* CardSlot = Root->AddChildToOverlay(CardSize);
        CardSlot->SetHorizontalAlignment(HAlign_Center);
        CardSlot->SetVerticalAlignment(VAlign_Center);

        UBorder* Card = WidgetTree->ConstructWidget<UBorder>();
        Card->SetBrushColor(FLinearColor(.035f, .044f, .065f, 1.f));
        Card->SetPadding(FMargin(36.f));
        CardSize->SetContent(Card);
        UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
        Card->SetContent(Column);

        auto AddText = [&](int32 FontSize, const FLinearColor& Color, float BottomPadding)
        {
            UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
            Text->SetFont(FCoreStyle::GetDefaultFontStyle(FontSize >= 20 ? "Bold" : "Regular", FontSize));
            Text->SetColorAndOpacity(FSlateColor(Color));
            Text->SetJustification(ETextJustify::Center);
            Text->SetAutoWrapText(true);
            Column->AddChildToVerticalBox(Text)->SetPadding(FMargin(0.f, 0.f, 0.f, BottomPadding));
            return Text;
        };

        Heading = AddText(34, FLinearColor(.91f, .77f, .48f), 16.f);
        Description = AddText(15, FLinearColor(.73f, .79f, .87f), 28.f);

        auto AddButton = [&](const FText& Label, bool bPrimary)
        {
            UButton* Button = WidgetTree->ConstructWidget<UButton>();
            FButtonStyle Style;
            Style.SetNormal(FSlateColorBrush(bPrimary ? FLinearColor(.29f, .23f, .13f) : FLinearColor(.09f, .12f, .17f)));
            Style.SetHovered(FSlateColorBrush(bPrimary ? FLinearColor(.43f, .34f, .18f) : FLinearColor(.15f, .20f, .28f)));
            Style.SetPressed(FSlateColorBrush(FLinearColor(.21f, .17f, .10f)));
            Style.SetNormalPadding(FMargin(18.f, 14.f));
            Style.SetPressedPadding(FMargin(18.f, 15.f, 18.f, 13.f));
            Button->SetStyle(Style);
            UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
            Text->SetText(Label);
            Text->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 17));
            Text->SetColorAndOpacity(FSlateColor(FLinearColor(.94f, .95f, .98f)));
            Text->SetJustification(ETextJustify::Center);
            Button->SetContent(Text);
            Column->AddChildToVerticalBox(Button)->SetPadding(FMargin(0.f, 0.f, 0.f, 10.f));
            return Button;
        };

        PrimaryButton = AddButton(FText::GetEmpty(), true);
        PrimaryLabel = Cast<UTextBlock>(PrimaryButton->GetContent());
        PrimaryButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnPrimaryClicked);
        TitleButton = AddButton(LOCTEXT("MainScreen", "메인 화면"), false);
        TitleButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnTitleClicked);
        UButton* QuitButton = AddButton(LOCTEXT("Quit", "게임 종료"), false);
        QuitButton->OnClicked.AddUniqueDynamic(this, &ThisClass::OnQuitClicked);
    }

    RefreshScreen();
    return Super::RebuildWidget();
}

void UDiaSessionMenuWidget::SetScreen(EDiaSessionMenuScreen InScreen)
{
    Screen = InScreen;
    RefreshScreen();
}

void UDiaSessionMenuWidget::RefreshScreen()
{
    if (!Heading || !Description || !PrimaryLabel || !TitleButton) return;
    TitleButton->SetVisibility(Screen == EDiaSessionMenuScreen::Title ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

    switch (Screen)
    {
    case EDiaSessionMenuScreen::Title:
        Heading->SetText(LOCTEXT("GameTitle", "ARPG"));
        Description->SetText(LOCTEXT("TitleDescription", "던전을 탐색하고 보스를 물리치세요.\nEsc 키로 메뉴를 열 수 있습니다."));
        PrimaryLabel->SetText(LOCTEXT("Start", "게임 시작"));
        break;
    case EDiaSessionMenuScreen::Pause:
        Heading->SetText(LOCTEXT("PauseTitle", "일시 정지"));
        Description->SetText(LOCTEXT("PauseDescription", "메인 화면으로 돌아가거나 종료하면\n이번 플레이의 진행 상황은 사라집니다."));
        PrimaryLabel->SetText(LOCTEXT("Resume", "계속하기"));
        break;
    case EDiaSessionMenuScreen::Cleared:
        Heading->SetText(LOCTEXT("ClearTitle", "던전 클리어"));
        Description->SetText(LOCTEXT("ClearDescription", "보스를 물리쳤습니다.\n플레이해 주셔서 감사합니다."));
        PrimaryLabel->SetText(LOCTEXT("Retry", "다시 시작"));
        break;
    case EDiaSessionMenuScreen::Defeated:
        Heading->SetText(LOCTEXT("DefeatTitle", "게임 오버"));
        Description->SetText(LOCTEXT("DefeatDescription", "이번 탐험은 여기까지입니다.\n새로운 던전에 다시 도전할 수 있습니다."));
        PrimaryLabel->SetText(LOCTEXT("Retry", "다시 시작"));
        break;
    }
}

TOptional<FUIInputConfig> UDiaSessionMenuWidget::GetDesiredInputConfig() const
{
    return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
}

UWidget* UDiaSessionMenuWidget::NativeGetDesiredFocusTarget() const
{
    return PrimaryButton;
}

FReply UDiaSessionMenuWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Escape)
    {
        if (Screen == EDiaSessionMenuScreen::Pause)
        {
            if (ADiaController* Controller = GetOwningPlayer<ADiaController>()) Controller->ResumeSession();
        }
        return FReply::Handled();
    }
    return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UDiaSessionMenuWidget::OnPrimaryClicked()
{
    if (ADiaController* Controller = GetOwningPlayer<ADiaController>())
    {
        if (Screen == EDiaSessionMenuScreen::Title) Controller->StartSession();
        else if (Screen == EDiaSessionMenuScreen::Pause) Controller->ResumeSession();
        else Controller->RestartSession();
    }
}

void UDiaSessionMenuWidget::OnTitleClicked()
{
    if (ADiaController* Controller = GetOwningPlayer<ADiaController>()) Controller->ReturnToSessionTitle();
}

void UDiaSessionMenuWidget::OnQuitClicked()
{
    if (ADiaController* Controller = GetOwningPlayer<ADiaController>()) Controller->QuitSession();
}

#undef LOCTEXT_NAMESPACE
