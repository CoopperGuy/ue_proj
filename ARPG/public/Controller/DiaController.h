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
class UDiaItemDebugWidget;
class UDiaMenuSystem;
struct FRewardData;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, ADiaBaseCharacter*);

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

	void ToggleInventoryVisibility(bool bVisible);
	void ToggleChracterStatusVisibility(bool bVisible);
	void ToggleSkillPanelVisibility(bool bVisible);

	void RegisteSkillOnQuickSlotWidget(int32 SkillID, int32 SlotIndex);
	void RegisteSkillPannelWidget(const USkillObject* SkillData);
	void RegisteSkillPannelWidget(const TArray<USkillObject*>&);

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

	UFUNCTION()
	void OnEquipItemProgress(const FEquippedItem& Item, EEquipmentSlot SlotType);
	UFUNCTION()
	void OnUnequipItemProgress(EEquipmentSlot SlotType);

	UFUNCTION(Exec)
	void ToggleItemDebugUI();



protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	void OnPossess(APawn* InPawn) override;

	bool ApplyGoldReward(const FRewardData& RewardData);
	bool ApplyItemReward(const FRewardData& RewardData);
	bool ApplySkillAddReward(const FRewardData& RewardData);
	bool ApplySkillUpgradeReward(const FRewardData& RewardData);
	bool ApplySkillVariantReward(const FRewardData& RewardData);

	void BindSkillManagerDelegates(APawn* InPawn);
	void HandleSkillRegistered(int32 SkillID, int32 SlotIndex);
	void HandleSkillLevelChanged(int32 SkillID, int32 NewLevel);
	void HandleSkillVariantAdded(int32 SkillID, int32 VariantID);

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
	UHUDWidget* GetHUDWidget() const;
	bool IsSkillInputBlocked() const;
	bool IsUIInputBlocked() const;
	UDiaMenuSystem* GetMenuSystemWidget() const;
	ESlateVisibility GetInventoryVisibility() const;
	ESlateVisibility GetWidgetVisibility(const FName& FoundName) const;
};
