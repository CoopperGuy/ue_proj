#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DiaSessionMenuWidget.generated.h"

class UButton;
class UTextBlock;

enum class EDiaSessionMenuScreen : uint8
{
    Title,
    Pause,
    Cleared,
    Defeated
};

// A native menu so the one-run demo needs no additional Widget Blueprint or title map.
UCLASS()
class ARPG_API UDiaSessionMenuWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UDiaSessionMenuWidget(const FObjectInitializer& ObjectInitializer);
    void SetScreen(EDiaSessionMenuScreen InScreen);
    EDiaSessionMenuScreen GetScreen() const { return Screen; }

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
    virtual UWidget* NativeGetDesiredFocusTarget() const override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    void RefreshScreen();

    UFUNCTION()
    void OnPrimaryClicked();
    UFUNCTION()
    void OnTitleClicked();
    UFUNCTION()
    void OnQuitClicked();

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> Heading;
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> Description;
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PrimaryLabel;
    UPROPERTY(Transient)
    TObjectPtr<UButton> PrimaryButton;
    UPROPERTY(Transient)
    TObjectPtr<UButton> TitleButton;

    EDiaSessionMenuScreen Screen = EDiaSessionMenuScreen::Title;
};
