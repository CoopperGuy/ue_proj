#include "Controller/DiaController.h"

#include "Engine/World.h"
#include "GameMode/DungeonGameMode.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Logging/ARPGLogChannels.h"
#include "System/GameViewPort/DiaCustomGameViewPort.h"
#include "TimerManager.h"
#include "UI/Menu/DiaSessionMenuWidget.h"
#include "UI/System/DiaPrimaryLayout.h"
#include "Widgets/SViewport.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/EditorEngine.h"
#endif

void ADiaController::InitializeSessionMenu()
{
    if (!IsLocalController() || !GetWorld() || !GetWorld()->GetAuthGameMode<ADungeonGameMode>()) return;

    const AGameModeBase* Mode = GetWorld()->GetAuthGameMode();
    if (UGameplayStatics::HasOption(Mode->OptionsString, TEXT("StartRun")))
    {
        ApplyInputRouting(EDiaInputRouting::GameAndUI);
        return;
    }
    ShowSessionMenu(EDiaSessionMenuScreen::Title);
}

void ADiaController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(SessionStartupTimer);
    if (IsValid(SessionMenuWidget))
    {
        SessionMenuWidget->DeactivateWidget();
        SessionMenuWidget->RemoveFromParent();
        SessionMenuWidget = nullptr;
    }
    Super::EndPlay(EndPlayReason);
}

bool ADiaController::IsSessionMenuOpen() const
{
    return IsValid(SessionMenuWidget) && SessionMenuWidget->IsInViewport() && SessionMenuWidget->IsActivated();
}

void ADiaController::ShowSessionMenu(EDiaSessionMenuScreen Screen)
{
    if (!IsLocalController() || bSessionTravelPending) return;
    if (!IsValid(SessionMenuWidget))
        SessionMenuWidget = CreateWidget<UDiaSessionMenuWidget>(this, UDiaSessionMenuWidget::StaticClass());
    if (!IsValid(SessionMenuWidget))
    {
        UE_LOG(LogARPG, Error, TEXT("Failed to create the session menu."));
        return;
    }

    SessionMenuWidget->SetScreen(Screen);
    if (UDiaCustomGameViewPort* Viewport = Cast<UDiaCustomGameViewPort>(GetWorld()->GetGameViewport()))
    {
        if (UDiaPrimaryLayout* Layout = Viewport->GetDiaPrimaryLayout())
        {
            Layout->CloseMenu();
            Layout->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
    if (ACharacter* PlayerCharacter = GetCharacter())
    {
        PlayerCharacter->ConsumeMovementInputVector();
        PlayerCharacter->GetCharacterMovement()->StopMovementImmediately();
    }

    if (!SessionMenuWidget->IsInViewport()) SessionMenuWidget->AddToViewport(2000);
    SessionMenuWidget->ActivateWidget();
    ApplyInputRouting(EDiaInputRouting::UIOnly);
    if (!IsPaused())
    {
        bSessionMenuPausedWorld = SetPause(true);
        if (!bSessionMenuPausedWorld)
            UE_LOG(LogARPG, Warning, TEXT("The session menu could not pause the world."));
    }

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(SessionMenuWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
    bShowMouseCursor = true;
    SessionMenuWidget->RequestRefreshFocus();
}

void ADiaController::HideSessionMenu()
{
    if (IsValid(SessionMenuWidget))
    {
        SessionMenuWidget->DeactivateWidget();
        SessionMenuWidget->RemoveFromParent();
    }
    if (bSessionMenuPausedWorld)
    {
        SetPause(false);
        bSessionMenuPausedWorld = false;
    }
    if (UDiaCustomGameViewPort* Viewport = Cast<UDiaCustomGameViewPort>(GetWorld()->GetGameViewport()))
    {
        if (UDiaPrimaryLayout* Layout = Viewport->GetDiaPrimaryLayout())
            Layout->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    ApplyInputRouting(EDiaInputRouting::GameAndUI);
    FInputModeGameAndUI InputMode;
    if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
        InputMode.SetWidgetToFocus(Viewport->GetGameViewportWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
    bShowMouseCursor = true;
}

void ADiaController::StartSession()
{
    if (!bSessionTravelPending && IsSessionMenuOpen() && SessionMenuWidget->GetScreen() == EDiaSessionMenuScreen::Title)
        HideSessionMenu();
}

void ADiaController::ResumeSession()
{
    if (!bSessionTravelPending && IsSessionMenuOpen() && SessionMenuWidget->GetScreen() == EDiaSessionMenuScreen::Pause)
        HideSessionMenu();
}

void ADiaController::ShowSessionResult(bool bCleared)
{
    if (IsSessionMenuOpen() && (SessionMenuWidget->GetScreen() == EDiaSessionMenuScreen::Cleared
        || SessionMenuWidget->GetScreen() == EDiaSessionMenuScreen::Defeated)) return;
    ShowSessionMenu(bCleared ? EDiaSessionMenuScreen::Cleared : EDiaSessionMenuScreen::Defeated);
}

void ADiaController::ReloadSessionLevel(bool bStartImmediately)
{
    if (bSessionTravelPending || !IsLocalController()) return;
    const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    if (LevelName.IsEmpty()) return;

    // Reload the current playable map to reset the pawn, inventory, floor and reward state.
    bSessionTravelPending = true;
    if (IsValid(SessionMenuWidget)) SessionMenuWidget->SetIsEnabled(false);
    if (bSessionMenuPausedWorld)
    {
        SetPause(false);
        bSessionMenuPausedWorld = false;
    }
    UGameplayStatics::OpenLevel(this, FName(*LevelName), true, bStartImmediately ? TEXT("StartRun=1") : TEXT(""));
}

void ADiaController::RestartSession()
{
    ReloadSessionLevel(true);
}

void ADiaController::ReturnToSessionTitle()
{
    ReloadSessionLevel(false);
}

void ADiaController::QuitSession()
{
#if WITH_EDITOR
    if (GetWorld() && GetWorld()->WorldType == EWorldType::PIE)
    {
        if (GEditor) GEditor->RequestEndPlayMap();
        return;
    }
#endif
    UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
