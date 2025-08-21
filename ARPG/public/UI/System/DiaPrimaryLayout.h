// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"

#include "DiaPrimaryLayout.generated.h"

class UCommonActivatableWidget;
class UCommonActivatableWidgetStack;
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

	UCommonActivatableWidgetStack* GetLayerByTag(const FGameplayTag InTag) const;
	UUserWidget* PushToHudLayer(const FGameplayTag InTag, UCommonActivatableWidget* Widget);
	void PopTopFromLayer(const FGameplayTag InTag);

	void InitLayers();
protected:
	void NativeConstruct() override;
protected:
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite, Category = "Layout")
	UCommonActivatableWidgetStack* HudLayer;
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite, Category = "Layout")
	UCommonActivatableWidgetStack* ModelLayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TMap<FGameplayTag, UCommonActivatableWidgetStack*> HudWidgets;

};
