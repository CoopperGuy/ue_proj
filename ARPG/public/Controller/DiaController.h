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
class UDiaSessionMenuWidget;
class UAbilitySystemComponent;
struct FRewardData;
enum class EDiaSessionMenuScreen : uint8;

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
	UDiaOptionManagerComponent* GetOptionManagerComponent() { return DiaOptionManagerComponent; }
	const UDiaOptionManagerComponent* GetOptionManagerComponent() const { return DiaOptionManagerComponent; }

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

	/** Give a skill and equip it. QuickSlot: 0 = automatic, 1-5 = replace that slot. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat|Skills")
	void CheatGiveSkill(int32 SkillID, int32 QuickSlot = 0);

	/** Give all player skills, bypassing the owned limit. Include non-player skills only when requested. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat|Skills")
	void CheatGiveAllSkills(bool bIncludeNonPlayerSkills = false);

	/** Print loaded skill IDs, names, ownership and quick slots. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat|Skills")
	void CheatListSkills();

	/** Give an assigned variant to an owned skill. Apply immediately unless bApply is false. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat|Skills")
	void CheatGiveSkillVariant(int32 SkillID, int32 VariantID, bool bApply = true);

	/** Give all assigned variants. SkillID 0 selects every owned skill. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat|Skills")
	void CheatGiveAllSkillVariants(int32 SkillID = 0, bool bApply = true);

	/** Clear applied variants while keeping ownership. SkillID 0 selects every owned skill. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat|Skills")
	void CheatClearSkillVariants(int32 SkillID = 0);

	/** List assigned variants and ownership/application state. SkillID 0 selects owned skills. */
	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat|Skills")
	void CheatListSkillVariants(int32 SkillID = 0);

	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat")
	void CheatDropItemWithOptions(FName ItemID, const FString& PrefixOptionKeys, const FString& SuffixOptionKeys, int32 Count = 1, int32 Level = 1);

	UFUNCTION(Exec, BlueprintCallable, Category = "Cheat")
	void CheatGiveItemWithOptions(FName ItemID, const FString& PrefixOptionKeys, const FString& SuffixOptionKeys, int32 Count = 1, int32 Level = 1);

	UFUNCTION(Exec, BlueprintCallable, Category = "Debug")
	void CheatValidateOptionRolls(FName ItemID, int32 Count = 1000, int32 Level = 1);

	void SetMenuSystemVisibility(bool bVisible);
	void ToggleMenuSystemVisibility();

	void StartSession();
	void ResumeSession();
	void RestartSession();
	void ReturnToSessionTitle();
	void QuitSession();
	void ShowSessionResult(bool bCleared);
	bool IsSessionMenuOpen() const;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	void InitializeSessionMenu();
	void ShowSessionMenu(EDiaSessionMenuScreen Screen);
	void HideSessionMenu();
	void ReloadSessionLevel(bool bStartImmediately);

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

	UPROPERTY(Transient)
	TObjectPtr<UDiaSessionMenuWidget> SessionMenuWidget;

	FTimerHandle SessionStartupTimer;
	bool bSessionMenuPausedWorld = false;
	bool bSessionTravelPending = false;

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
