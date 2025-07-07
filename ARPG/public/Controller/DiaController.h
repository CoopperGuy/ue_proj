// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Types/ItemBase.h"
#include "DiaController.generated.h"


class UDiaInventoryComponent;
class UHUDWidget;
/**
 * 
 */
UCLASS()
class ARPG_API ADiaController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ADiaController();

	bool ItemAddedToInventory(const FInventoryItem& Item);
	void ItemRemoved(const FInventoryItem& Item);

	void ToggleInventoryVisibility(bool bVisible);
	void ToggleChracterStatusVisibility(bool bVisible);

	ESlateVisibility GetInventoryVisibility() const;
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDiaInventoryComponent* DiaInventoryComponent;

	UPROPERTY()
	mutable TWeakObjectPtr<UHUDWidget> CachedHUDWidget = nullptr;

private:
	// HUDWidget을 가져오는 헬퍼 함수 (최초 1회만 GameMode에서 가져옴)
	UHUDWidget* GetHUDWidget() const;
};
