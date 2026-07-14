// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DiaComponent/Skill/DiaSkillDelegates.h"
#include "Types/DiaGASSkillData.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect.h"
#include "DiaSkillManagerComponent.generated.h"


class USkillObject;
class UDiaSkillVariant;
class UDiaGameplayAbility;
class UDiaSkillLoadService;
class UDiaSkillActivationService;
class UDiaSkillVariantExecutorService;
class UDiaSkillVariantCache;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillRegisteredDelegate, int32 /*SkillID*/, int32 /*SlotIndex*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillLevelChangedDelegate, int32 /*SkillID*/, int32 /*NewLevel*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSkillVariantAddedDelegate, int32 /*SkillID*/, int32 /*VariantID*/);

UCLASS()
class ARPG_API UDiaSkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaSkillManagerComponent();

	static constexpr int32 MaxOwnedSkillCount = 8;
	static constexpr int32 MaxQuickSlotCount = 5;

	FOnSkillRegisteredDelegate OnSkillRegistered;
	FOnSkillLevelChangedDelegate OnSkillLevelChanged;
	FOnSkillVariantAddedDelegate OnSkillVariantAdded;

protected:
	virtual void BeginPlay() override;
public:
	void MakeSkillVariantsArray(IN const UDiaGameplayAbility* Ability ,OUT TArray<UDiaSkillVariant*>& OutVariantsArray);	
	void LoadJobSKillDataFromTable(EJobType JobType, int32 MaxSkillCount = MaxOwnedSkillCount);

	const int32 GetMappedSkillID(int32 Index) const;
	const int32 GetIndexOfSkillID(int32 SkillID) const;
	const USkillObject* GetSkillObjectBySkillID(int32 SkillID) const;
	const TArray<USkillObject*>& GetSkillIDMapping() const;
	const TArray<int32>& GetQuickSlotSkillIDs() const;
	const TArray<int32>& GetCurrentJobSkillIDs() const { return CurrentJobSkillSet.SkillIDs; }
	int32 FindFirstEmptyQuickSlotIndex() const;
	bool HasAvailableSkillSlot() const;
	bool TryRegisterSkillByID(int32 SkillID);
	bool RemoveSkillByID(int32 SkillID);
	void SetSkillIDMapping(const TArray<int32>& NewMapping);
	void SetSkillIDIndex(int32 SkillID, int32 Index);
	void NotifySkillRegistered(int32 SkillID, int32 SlotIndex);

	bool AddOwnedVariantBySkillID(int32 SkillID, int32 VariantID);
	bool TryUnlockSkillVariantByID(int32 SkillID, int32 VariantID);
	bool TryApplySkillVariantByID(int32 SkillID, int32 VariantID);
	// Try activate ability by skill ID
	UFUNCTION(BlueprintCallable, Category = "GAS|Skills")
	bool TryActivateAbilityBySkillID(int32 SkillID);

	bool TryAddSkillLevelBySkillID(int32 SkillID, int32 LevelToAdd);

	// Get ability spec by skill ID (C++ only - not exposed to Blueprint)
	FGameplayAbilitySpec* GetAbilitySpecBySkillID(int32 SkillID) const;

	void ActiveModifierSkillUseVariants(const FDiaSkillVariantContext& context, const UDiaGameplayAbility* Ability, FSkillModifierRuntime& OutRuntime);
	void SpawnSkillActorUseVariants(FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability, FDiaSkillSpawnFinishedDelegate OnFinished = FDiaSkillSpawnFinishedDelegate());
	void HitSkillActorUseVariants(const FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability);
	void HitSkillActorUseVariants(const FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability, FSkillHitRuntime& OutRuntime);
	
	void GetSkillVariantFromID(const int32 VariantID, OUT UDiaSkillVariant* OutVariants);
	void GetSkillVariantsFromSkillID(const int32 SkillID, OUT TArray<UDiaSkillVariant*>& OutVariants);
	void GetOwnedSkillVariantsFromSkillID(const int32 SkillID, OUT TArray<UDiaSkillVariant*>& OutVariants);

protected:
	USkillObject* GetMutableSkillObjectBySkillID(int32 SkillID);
protected:
	FJobSkillSet CurrentJobSkillSet;

	//스킬 관련 변수
    // 스킬 ID 매핑 (키 인덱스 -> 스킬 ID)
	UPROPERTY(EditDefaultsOnly, Category = "Skills")
	TArray<USkillObject*> SkillIDMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Skills")
	TArray<int32> QuickSlotSkillIDs;

	UPROPERTY()
	TMap<int32, UDiaSkillVariant*> SkillVariants;

	// Service 인스턴스
	UPROPERTY()
	UDiaSkillLoadService* LoadService;

	UPROPERTY()
	UDiaSkillActivationService* ActivationService;

	UPROPERTY()
	UDiaSkillVariantExecutorService* VariantExecutor;

	UPROPERTY()
	UDiaSkillVariantCache* VariantCache;
};

