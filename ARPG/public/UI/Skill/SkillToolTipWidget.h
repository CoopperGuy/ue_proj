#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillToolTipWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

enum class ESkillToolTipContext : uint8
{
    SkillList,
    VariantList,
    QuickSlot
};

// Built in C++ so every skill slot gets the same card without a Blueprint class assignment.
UCLASS()
class ARPG_API USkillToolTipWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    bool SetSkill(int32 SkillID, ESkillToolTipContext Context, int32 MainSkillID = INDEX_NONE, int32 FallbackLevel = 1);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void RefreshContent();

    UPROPERTY(Transient)
    TObjectPtr<UImage> IconImage;
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> TitleText;
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> SubtitleText;
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> BodyText;
    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> HintText;
    UPROPERTY(Transient)
    TObjectPtr<UTexture2D> SkillIcon;

    FText Title;
    FText Subtitle;
    FText Body;
    FText Hint;
    FLinearColor Accent = FLinearColor(.88f, .69f, .36f);
};
