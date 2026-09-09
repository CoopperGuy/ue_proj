// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include "Logging/ARPGLogChannels.h"
#include "DiaPrimaryLayout.generated.h"

class UHUDWidget;
class UDiaMainMenuWorkspace;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaPrimaryLayout : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout")
	FGameplayTag DefaultHudTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout")
	FGameplayTag DefaultModelTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Layout")
	FGameplayTag DefaultMenuTag;

	UCommonActivatableWidgetStack* GetLayerByTag(const FGameplayTag& InTag) const;
	void PopTopFromLayer(const FGameplayTag& InTag);
	UCommonActivatableWidget* PushToLayer(const FGameplayTag& InTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);
	UCommonActivatableWidget* OpenMenuPage(const FGameplayTag& UITag);
	void CloseMenu();
	UCommonActivatableWidget* GetTopWidgetFromLayer(const FGameplayTag& InTag) const;
	UHUDWidget* GetHUDWidget() const;

	bool ToggleMenu(const FGameplayTag& UITag);

	void InitLayers();
	void DisableLayers();

	bool IsMainMenuOpen() const;
protected:
	void NativeConstruct() override;
	UDiaMainMenuWorkspace* GetActiveMenuWorkspace() const;
	UDiaMainMenuWorkspace* GetOrCreateMenuWorkspace();
	void SetHUDInteractionEnabled(bool bEnabled);
protected:
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite, Category = "Layout")
	UCommonActivatableWidgetStack* HudLayer;
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite, Category = "Layout")
	UCommonActivatableWidgetStack* ModalLayer;
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite, Category = "Layout")
	UCommonActivatableWidgetStack* MenuLayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TMap<FGameplayTag, UCommonActivatableWidgetStack*> HudWidgets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<UCommonActivatableWidget> DefaultHudWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<UCommonActivatableWidget> DefaultMenuWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UHUDWidget> HUDWidget;
};
