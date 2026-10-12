#include "UI/Skill/SkillToolTipWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Pawn.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"
#include "Styling/CoreStyle.h"
#include "System/GASSkillManager.h"

namespace
{
    const TCHAR* SkillTypeName(EGASSkillType Type)
    {
        switch (Type)
        {
        case EGASSkillType::MeleeAttack: return TEXT("근접 공격");
        case EGASSkillType::RangedAttack: return TEXT("원거리 공격");
        case EGASSkillType::Magic: return TEXT("마법");
        case EGASSkillType::Buff: return TEXT("강화");
        case EGASSkillType::Debuff: return TEXT("약화");
        case EGASSkillType::Heal: return TEXT("치유");
        case EGASSkillType::Movement: return TEXT("이동");
        case EGASSkillType::Dodge: return TEXT("회피");
        case EGASSkillType::Passive: return TEXT("지속 효과");
        default: return TEXT("스킬");
        }
    }

    FString DescriptionOrFallback(const FText& Description)
    {
        const FString Text = Description.ToString().TrimStartAndEnd();
        return Text.IsEmpty() || Text == TEXT("No Description") ? TEXT("등록된 설명이 없습니다.") : Text;
    }
}

bool USkillToolTipWidget::SetSkill(int32 SkillID, ESkillToolTipContext Context, int32 MainSkillID, int32 FallbackLevel)
{
    // A recycled entry must never retain the previous skill's image or description.
    Title = Subtitle = Body = Hint = FText::GetEmpty();
    SkillIcon = nullptr;
    Accent = FLinearColor(.88f, .69f, .36f);

    UGASSkillManager* Data = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
    APawn* Pawn = GetOwningPlayerPawn();
    const UDiaSkillManagerComponent* Skills = Pawn ? Pawn->FindComponentByClass<UDiaSkillManagerComponent>() : nullptr;
    const bool bVariant = Context == ESkillToolTipContext::VariantList;
    const USkillObject* OwnedSkill = Skills ? Skills->GetSkillObjectBySkillID(bVariant ? MainSkillID : SkillID) : nullptr;
    if (!Data || SkillID <= 0)
    {
        RefreshContent();
        return false;
    }

    if (bVariant)
    {
        const FSkillVariantData* Variant = Data->GetSkllVariantDataPtr(SkillID);
        if (!Variant)
        {
            RefreshContent();
            return false;
        }
        const FGASSkillData* Parent = Data->GetSkillDataPtr(MainSkillID);
        const bool bApplied = OwnedSkill && OwnedSkill->GetVariantApplyIDs().Contains(SkillID);
        Title = Variant->VariantName;
        Subtitle = FText::FromString(bApplied ? TEXT("스킬 변형 · 적용 중") : TEXT("스킬 변형 · 미적용"));
        SkillIcon = Variant->Icon.LoadSynchronous();
        Accent = FLinearColor(.64f, .53f, 1.f);
        FString Text = DescriptionOrFallback(Variant->Description);
        if (Parent) Text += FString::Printf(TEXT("\n\n적용 대상: %s"), *Parent->SkillName.ToString());
        Body = FText::FromString(Text);
        Hint = FText::FromString(TEXT("적용: 이 스킬에 변형 효과 추가\n해제: 변형 효과 해제"));
    }
    else
    {
        const FGASSkillData* Skill = Data->GetSkillDataPtr(SkillID);
        if (!Skill)
        {
            RefreshContent();
            return false;
        }
        Title = Skill->SkillName;
        const int32 Level = OwnedSkill ? OwnedSkill->GetSkillLevel() : FallbackLevel;
        Subtitle = FText::FromString(FString::Printf(TEXT("%s · 레벨 %d / %d"), SkillTypeName(Skill->SkillType),
            FMath::Max(1, Level), FMath::Max(1, Skill->MaxLevel)));
        SkillIcon = Skill->Icon.LoadSynchronous();

        // Preview active cost modifiers from equipped variants, without activating the ability.
        float ManaMultiplier = 1.f;
        float CooldownMultiplier = 1.f;
        TArray<FString> AppliedNames;
        if (OwnedSkill)
        {
            TArray<int32> AppliedIDs = OwnedSkill->GetVariantApplyIDs().Array();
            AppliedIDs.Sort();
            for (int32 VariantID : AppliedIDs)
            {
                const FSkillVariantData* Variant = Data->GetSkllVariantDataPtr(VariantID);
                if (!Variant) continue;
                AppliedNames.Add(Variant->VariantName.ToString());
                if (Variant->VariantTag == FDiaGameplayTags::Get().GASData_Variant_ManaCostMultiplier)
                    ManaMultiplier *= Variant->ModifierValue;
                if (Variant->VariantTag == FDiaGameplayTags::Get().GASData_Variant_CooldownReduction)
                    CooldownMultiplier *= Variant->ModifierValue;
            }
        }
        const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
        const float Reduction = ASC ? ASC->GetNumericAttribute(UDiaAttributeSet::GetCooldownReductionAttribute()) : 0.f;
        const float Mana = FMath::Max(0.f, Skill->ManaCost * ManaMultiplier);
        const float Cooldown = FMath::Max(0.f, Skill->CooldownDuration * CooldownMultiplier * FMath::Clamp(1.f - Reduction * .01f, 0.f, 1.f));

        FString Text = DescriptionOrFallback(Skill->Description) + TEXT("\n");
        if (Skill->BaseDamage > 0.f) Text += FString::Printf(TEXT("\n기본 피해량  %.1f"), Skill->BaseDamage);
        Text += FString::Printf(TEXT("\n마나 소모  %.1f\n재사용 대기시간  %.1f초"), Mana, Cooldown);
        if (Skill->CastTime > 0.f) Text += FString::Printf(TEXT("\n시전 시간  %.1f초"), Skill->CastTime);
        if (!FMath::IsNearlyEqual(Mana, Skill->ManaCost) || !FMath::IsNearlyEqual(Cooldown, Skill->CooldownDuration))
            Text += FString::Printf(TEXT("\n기본값: 마나 %.1f · 재사용 %.1f초"), Skill->ManaCost, Skill->CooldownDuration);
        if (!AppliedNames.IsEmpty())
        {
            const int32 Total = AppliedNames.Num();
            if (Total > 4) AppliedNames.SetNum(4);
            Text += TEXT("\n\n적용 중인 변형\n") + FString::Join(AppliedNames, TEXT(", "));
            if (Total > 4) Text += FString::Printf(TEXT(" 외 %d개"), Total - 4);
        }
        Body = FText::FromString(Text);
        Hint = FText::FromString(Context == ESkillToolTipContext::QuickSlot
            ? TEXT("스킬창에서 등록과 해제를 할 수 있습니다.")
            : TEXT("적용: 빈 퀵슬롯에 등록\n해제: 퀵슬롯 등록 해제"));
    }
    RefreshContent();
    return true;
}

TSharedRef<SWidget> USkillToolTipWidget::RebuildWidget()
{
    if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
    if (!WidgetTree->RootWidget)
    {
        USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("TooltipSize"));
        Size->SetWidthOverride(352.f);
        WidgetTree->RootWidget = Size;
        UBorder* Frame = WidgetTree->ConstructWidget<UBorder>();
        Frame->SetBrushColor(FLinearColor(.27f, .22f, .14f, 1.f));
        Frame->SetPadding(FMargin(1.f));
        Size->SetContent(Frame);
        UBorder* Background = WidgetTree->ConstructWidget<UBorder>();
        Background->SetBrushColor(FLinearColor(.018f, .022f, .03f, .98f));
        Background->SetPadding(FMargin(14.f));
        Frame->SetContent(Background);
        UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>();
        Background->SetContent(Column);
        UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>();
        Column->AddChildToVerticalBox(Header)->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
        USizeBox* IconSize = WidgetTree->ConstructWidget<USizeBox>();
        IconSize->SetWidthOverride(48.f);
        IconSize->SetHeightOverride(48.f);
        Header->AddChildToHorizontalBox(IconSize)->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
        IconImage = WidgetTree->ConstructWidget<UImage>();
        IconSize->SetContent(IconImage);
        UVerticalBox* Titles = WidgetTree->ConstructWidget<UVerticalBox>();
        UHorizontalBoxSlot* TitleSlot = Header->AddChildToHorizontalBox(Titles);
        TitleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        TitleSlot->SetVerticalAlignment(VAlign_Center);

        auto AddText = [&](UVerticalBox* Parent, int32 FontSize, const FLinearColor& Color, float TopPadding)
        {
            UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>();
            Text->SetFont(FCoreStyle::GetDefaultFontStyle(FontSize >= 16 ? "Bold" : "Regular", FontSize));
            Text->SetColorAndOpacity(FSlateColor(Color));
            Text->SetAutoWrapText(true);
            Parent->AddChildToVerticalBox(Text)->SetPadding(FMargin(0.f, TopPadding, 0.f, 0.f));
            return Text;
        };
        TitleText = AddText(Titles, 16, Accent, 0.f);
        SubtitleText = AddText(Titles, 11, FLinearColor(.65f, .71f, .8f), 4.f);
        BodyText = AddText(Column, 12, FLinearColor(.88f, .9f, .94f), 0.f);
        BodyText->SetWrapTextAt(320.f);
        HintText = AddText(Column, 10, FLinearColor(.51f, .57f, .66f), 12.f);
    }
    SetVisibility(ESlateVisibility::HitTestInvisible);
    RefreshContent();
    return Super::RebuildWidget();
}

void USkillToolTipWidget::RefreshContent()
{
    if (TitleText) { TitleText->SetText(Title); TitleText->SetColorAndOpacity(FSlateColor(Accent)); }
    if (SubtitleText) SubtitleText->SetText(Subtitle);
    if (BodyText) BodyText->SetText(Body);
    if (HintText) HintText->SetText(Hint);
    if (IconImage)
    {
        IconImage->SetBrushFromTexture(SkillIcon);
        IconImage->SetVisibility(SkillIcon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
    }
}
