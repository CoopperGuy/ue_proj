// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Types/ItemBase.h"
#include "Types/DiaItemOptionRow.h"
#include "DiaController.generated.h"



class ADiaMonster;
class UDiaInventoryComponent;
class UDiaEquipmentComponent;
class UDiaOptionManagerComponent;
class UHUDWidget;
class UDiaStatComponent;
class ADiaBaseCharacter;
class USkillObject;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, ADiaBaseCharacter*);

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
	void ToggleSkillPanelVisibility(bool bVisible);

	void RegisteSkillOnQuickSlotWidget(int32 SkillID, int32 SlotIndex);
	void RegisteSkillPannelWidget(const TArray<USkillObject*>&);

	ESlateVisibility GetInventoryVisibility() const;
	ESlateVisibility GetWidgetVisibility(const FName& FoundName) const;

	// StatComponent 초기화 관련 함수들
	//UFUNCTION()
	//void OnStatComponentInitialized(UDiaStatComponent* StatComponent);
	//
	//void BindUIToStatComponent(UDiaStatComponent* StatComponent);

	void SetTarget(ADiaBaseCharacter* NewTarget);

	UFUNCTION()
	void OnEquipItemProgress(const FEquippedItem& Item, EEquipmentSlot SlotType);
	UFUNCTION()
	void OnUnequipItemProgress(EEquipmentSlot SlotType);
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void OnPossess(APawn* InPawn) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDiaInventoryComponent* DiaInventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDiaEquipmentComponent* DiaEquipmentComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDiaOptionManagerComponent* DiaOptionManagerComponent;

	UPROPERTY()
	mutable TWeakObjectPtr<UHUDWidget> CachedHUDWidget = nullptr;

	FOnTargetChanged OnTargetChanged;

	UPROPERTY()
	ADiaBaseCharacter* TargetMonster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UHUDWidget> HUDWidgetClass;

public:
	FORCEINLINE FOnTargetChanged& GetOnTargetChanged() { return OnTargetChanged; }
	UHUDWidget* GetHUDWidget() const;
};
