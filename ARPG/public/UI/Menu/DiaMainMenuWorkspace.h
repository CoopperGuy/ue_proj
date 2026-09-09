// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "DiaMainMenuWorkspace.generated.h"

class UCommonActivatableWidgetSwitcher;

UENUM(BlueprintType)
enum class EDiaMenuPageRegion : uint8
{
    Navigation,
    Content
};

USTRUCT(BlueprintType)
struct FDiaMenuPageDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly)
    EDiaMenuPageRegion Region = EDiaMenuPageRegion::Content;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UCommonActivatableWidget> WidgetClass;
};
/**
 * Menu-layer workspace that owns the left navigation and right content slots.
 */
UCLASS(Abstract, Blueprintable)
class ARPG_API UDiaMainMenuWorkspace : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UDiaMainMenuWorkspace(const FObjectInitializer& ObjectInitializer);

	UCommonActivatableWidget* ShowPage(const FGameplayTag& InTag);
	bool IsPageActive(const FGameplayTag& InTag) const;

	UCommonActivatableWidget* GetActiveNavigation() const;
	UCommonActivatableWidget* GetActiveContent() const;
protected:
	virtual void NativeConstruct() override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
protected:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Main Menu Workspace")
	TObjectPtr<UCommonActivatableWidgetSwitcher> NavigationStack;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Main Menu Workspace")
	TObjectPtr<UCommonActivatableWidgetSwitcher> ContentStack;

	UPROPERTY(EditDefaultsOnly, Category = "Menu", meta = (Categories = "UI.MenuPage"))
	TMap<FGameplayTag, FDiaMenuPageDefinition> MenuPages;

	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidget>> ActiveMenuPages;

	UPROPERTY(Transient)
	FGameplayTag ActiveNavigationTag;

	UPROPERTY(Transient)
	FGameplayTag ActiveContentTag;
};
