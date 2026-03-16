// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include "DiaPrimaryLayout.generated.h"

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
	void PopTopFromLayer(const FGameplayTag InTag);

	void InitLayers();
	void DisableLayers();
protected:
	void NativeConstruct() override;
protected:
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite, Category = "Layout")
	UCommonActivatableWidgetStack* HudLayer;
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite, Category = "Layout")
	UCommonActivatableWidgetStack* ModelLayer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TMap<FGameplayTag, UCommonActivatableWidgetStack*> HudWidgets;


public:
	template<class T>
	T* PushToHudLayer(const FGameplayTag InTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
	{
		if (!this->IsInViewport())
		{
			this->AddToViewport();
		}

		UE_LOG(LogTemp, Warning, TEXT("PushToHudLayer called with tag: %s"), *InTag.ToString());
		if (!WidgetClass)
		{
			return nullptr;
		}

		UCommonActivatableWidgetStack* Layer = GetLayerByTag(InTag);
		if (!Layer)
		{
			UE_LOG(LogTemp, Warning, TEXT("PushToHudLayer No layer found for tag: %s"), *InTag.ToString());
			return nullptr;
		}

		T* Widget = Cast<T>(Layer->AddWidget(WidgetClass));
		if(!Widget)
		{
			UE_LOG(LogTemp, Warning, TEXT("PushToHudLayer Failed to create widget of class: %s"), *WidgetClass->GetName());
			return nullptr;
		}

		// 레이어 가시성 확보
		Layer->SetVisibility(ESlateVisibility::Visible);

		// 위젯 가시성 설정
		Widget->SetVisibility(ESlateVisibility::Visible);


		UCommonActivatableWidget* Active = Layer->GetActiveWidget();
		UE_LOG(LogTemp, Warning, TEXT("ActiveWidget after AddWidget: %s"),
			Active ? *Active->GetName() : TEXT("NULL"));
		UE_LOG(LogTemp, Warning, TEXT("Widget valid: %s, InViewport: %s"),
			Widget ? TEXT("YES") : TEXT("NO"),
			this->IsInViewport() ? TEXT("YES") : TEXT("NO"));


		// 이미 이 위젯이 레이어에 있는지 확인
		if (Layer->GetActiveWidget() == Widget)
		{
			UE_LOG(LogTemp, Warning, TEXT("PushToHudLayer Widget of class %s is already active in layer with tag: %s"), *WidgetClass->GetName(), *InTag.ToString());
			return Widget;
		}



	
		UE_LOG(LogTemp, Warning, TEXT("PushToHudLayer Widget of class %s pushed to layer with tag: %s"), *WidgetClass->GetName(), *InTag.ToString());
		return Widget;
	}
};
