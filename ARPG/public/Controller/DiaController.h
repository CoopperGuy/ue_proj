// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
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
class UDiaItemDebugWidget;
class UDiaMenuSystem;
class UAbilitySystemComponent;
struct FRewardData;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, ADiaBaseCharacter*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerASCReady, ADiaBaseCharacter*, UAbilitySystemComponent*);

UENUM(BlueprintType)
enum class EDiaInputRouting : uint8
{
	GameOnly UMETA(DisplayName = "Game Only"),
	UIOnly UMETA(DisplayName = "UI Only"),
	GameAndUI UMETA(DisplayName = "Game and UI")
};

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
	bool ApplyReward(const FRewardData& RewardData);
	void ApplyInputRouting(EDiaInputRouting  NewMode);

	bool ToggleMenuPage(const FGameplayTag& MenuPageTag);
	bool ToggleInventoryVisibility();
	bool ToggleCharacterStatusVisibility();
	bool ToggleSkillPanelVisibility();

	void RegisteSkillOnQuickSlotWidget(int32 SkillID, int32 SlotIndex);

	bool HandlePrimaryClick();


	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat")
	void CheatDropItem(FName ItemID, int32 Count = 1, int32 Level = 1);

	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat")
	void CheatGiveItem(FName ItemID, int32 Count = 1, int32 Level = 1);

	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat")
	void CheatDropItemWithOptions(FName ItemID, const FString& PrefixOptionKeys, const FString& SuffixOptionKeys, int32 Count = 1, int32 Level = 1);

	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat")
	void CheatGiveItemWithOptions(FName ItemID, const FString& PrefixOptionKeys, const FString& SuffixOptionKeys, int32 Count = 1, int32 Level = 1);

	UFUNCTION(Exec, BlueprintCallable, Category = "Debug")
	void CheatValidateOptionRolls(FName ItemID, int32 Count = 1000, int32 Level = 1);

	void SetMenuSystemVisibility(bool bVisible);
	void ToggleMenuSystemVisibility();
	// StatComponent 초기화 관련 함수들
	//UFUNCTION()
	//void OnStatComponentInitialized(UDiaStatComponent* StatComponent);
	//
	//void BindUIToStatComponent(UDiaStatComponent* StatComponent);

	void SetTarget(ADiaBaseCharacter* NewTarget);
	void NotifyPlayerASCReady(ADiaBaseCharacter* PlayerCharacter, UAbilitySystemComponent* AbilitySystem);

	UFUNCTION()
	void OnEquipItemProgress(const FEquippedItem& Item, EEquipmentSlot SlotType);
	UFUNCTION()
	void OnUnequipItemProgress(EEquipmentSlot SlotType);

	UFUNCTION(Exec)
	void ToggleItemDebugUI();



protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	bool ApplyGoldReward(const FRewardData& RewardData);
	bool ApplyItemReward(const FRewardData& RewardData);
	bool ApplySkillAddReward(const FRewardData& RewardData);
	bool ApplySkillUpgradeReward(const FRewardData& RewardData);
	bool ApplySkillVariantReward(const FRewardData& RewardData);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDiaInventoryComponent* DiaInventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDiaEquipmentComponent* DiaEquipmentComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDiaOptionManagerComponent* DiaOptionManagerComponent;

	UPROPERTY()
	mutable TWeakObjectPtr<UHUDWidget> CachedHUDWidget = nullptr;

	FOnTargetChanged OnTargetChanged;
	FOnPlayerASCReady OnPlayerASCReady;

	UPROPERTY()
	ADiaBaseCharacter* TargetMonster;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UHUDWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	TSubclassOf<UDiaItemDebugWidget> ItemDebugWidgetClass;

	UPROPERTY()
	TObjectPtr<UDiaItemDebugWidget> ItemDebugWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	TSubclassOf<UDiaMenuSystem> MenuSystemClass;
	
	UPROPERTY()
	TObjectPtr<UDiaMenuSystem> MenuSystemWidget;

	UPROPERTY()
	EDiaInputRouting CurrentUIMode = EDiaInputRouting::GameOnly;

public:
	FORCEINLINE FOnTargetChanged& GetOnTargetChanged() { return OnTargetChanged; }
	FORCEINLINE FOnPlayerASCReady& GetOnPlayerASCReady() { return OnPlayerASCReady; }
	UHUDWidget* GetHUDWidget() const;
	UDiaInventoryComponent* GetInventoryComponent() const { return DiaInventoryComponent; }
	UDiaEquipmentComponent* GetEquipmentComponent() const { return DiaEquipmentComponent; }
	bool IsSkillInputBlocked() const;
	bool IsUIInputBlocked() const;
	UDiaMenuSystem* GetMenuSystemWidget() const;
};
