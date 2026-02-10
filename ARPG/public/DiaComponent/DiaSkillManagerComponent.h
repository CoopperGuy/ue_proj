// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/DiaGASSkillData.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaSkillManagerComponent.generated.h"


class USkillObject;
class UDiaSkillVariant;
class UDiaGameplayAbility;
class UDiaSkillLoadService;
class UDiaSkillActivationService;
class UDiaSkillVariantExecutorService;
class UDiaSkillVariantCache;

UCLASS()
class ARPG_API UDiaSkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaSkillManagerComponent();

protected:
	virtual void BeginPlay() override;
public:
	void MakeSkillVariantsArray(IN const UDiaGameplayAbility* Ability ,OUT TArray<UDiaSkillVariant*>& OutVariantsArray);	
	void LoadJobSKillDataFromTable(EJobType JobType);

	const int32 GetMappedSkillID(int32 Index) const;
	const int32 GetIndexOfSkillID(int32 SkillID) const;
	const USkillObject* GetSkillObjectBySkillID(int32 SkillID) const;
	const TArray<USkillObject*>& GetSkillIDMapping() const;
	const TArray<int32>& GetCurrentJobSkillIDs() const { return CurrentJobSkillSet.SkillIDs; }
	void SetSkillIDMapping(const TArray<int32>& NewMapping);
	void SetSkillIDIndex(int32 SkillID, int32 Index);

	void AddVariantBySkillId(int32 SkillID, int32 VariantID);
	// Try activate ability by skill ID
	UFUNCTION(BlueprintCallable, Category = "GAS|Skills")
	bool TryActivateAbilityBySkillID(int32 SkillID);

	// Get ability spec by skill ID (C++ only - not exposed to Blueprint)
	FGameplayAbilitySpec* GetAbilitySpecBySkillID(int32 SkillID) const;

	void SpawnSkillActorUseVariants(const FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability);
	void HitSkillActorUseVariants(const FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability);

	void GetSkillVariantFromID(const int32 VariantID, OUT UDiaSkillVariant* OutVariants);
	void GetSkillVariantsFromSkillID(const int32 SkillID, OUT TArray<UDiaSkillVariant*>& OutVariants);
protected:
	FJobSkillSet CurrentJobSkillSet;

	//스킬 관련 변수
    // 스킬 ID 매핑 (키 인덱스 -> 스킬 ID)
	UPROPERTY(EditDefaultsOnly, Category = "Skills")
	TArray<USkillObject*> SkillIDMapping;

	UPROPERTY()
	TMap<int32, UDiaSkillVariant*> SkillVariants;
	const int32 MaxSkillMapping = 8;

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

