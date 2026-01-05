// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/DiaGASSkillData.h"
#include "DiaSkillManagerComponent.generated.h"


class USkillObject;
UCLASS()
class ARPG_API UDiaSkillManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaSkillManagerComponent();

protected:
	virtual void BeginPlay() override;

public:	
	void LoadJobSKillDataFromTable(EJobType JobType);

	const int32 GetMappedSkillID(int32 Index) const;
	const int32 GetIndexOfSkillID(int32 SkillID) const;
	const USkillObject* GetSkillObjectBySkillID(int32 SkillID) const;
	const TArray<USkillObject*>& GetSkillIDMapping() const;
	const TArray<int32>& GetCurrentJobSkillIDs() const { return CurrentJobSkillSet.SkillIDs; }
	void SetSkillIDMapping(const TArray<int32>& NewMapping);

	// Try activate ability by skill ID
	UFUNCTION(BlueprintCallable, Category = "GAS|Skills")
	bool TryActivateAbilityBySkillID(int32 SkillID);

	// Get ability spec by skill ID (C++ only - not exposed to Blueprint)
	FGameplayAbilitySpec* GetAbilitySpecBySkillID(int32 SkillID) const;
protected:
	FJobSkillSet CurrentJobSkillSet;

	//스킬 관련 변수
    // 스킬 ID 매핑 (키 인덱스 -> 스킬 ID)
	UPROPERTY(EditDefaultsOnly, Category = "Skills")
	TArray<USkillObject*> SkillIDMapping;

	const int32 MaxSkillMapping = 8;

};

