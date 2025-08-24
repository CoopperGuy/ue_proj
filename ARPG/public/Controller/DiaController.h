// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Types/ItemBase.h"
#include "DiaController.generated.h"


class UDiaInventoryComponent;
class UDiaEquipmentComponent;
class UHUDWidget;
class UDiaStatComponent;
/**
 * 
 */
UCLASS()
class ARPG_API ADiaController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ADiaController();

	bool ItemAddedToInventory(const FInventorySlot& Item);
	void ItemRemoved(const FInventorySlot& Item);

	void ToggleInventoryVisibility(bool bVisible);
	void ToggleChracterStatusVisibility(bool bVisible);

	ESlateVisibility GetInventoryVisibility() const;
	ESlateVisibility GetWidgetVisibility(const FName& FoundName) const;

	// StatComponent 초기화 관련 함수들
	UFUNCTION()
	void OnStatComponentInitialized(UDiaStatComponent* StatComponent);
	
	void BindUIToStatComponent(UDiaStatComponent* StatComponent);
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDiaInventoryComponent* DiaInventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDiaEquipmentComponent* DiaEquipmentComponent;

	UPROPERTY()
	mutable TWeakObjectPtr<UHUDWidget> CachedHUDWidget = nullptr;

private:
	// HUDWidget을 가져오는 헬퍼 함수 (최초 1회만 GameMode에서 가져옴)
	UHUDWidget* GetHUDWidget() const;

};
