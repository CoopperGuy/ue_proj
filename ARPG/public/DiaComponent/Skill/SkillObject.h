// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Types/DiaGASSkillData.h"
#include "SkillObject.generated.h"

class UDiaGameplayAbility;
class UDiaSkillVariant;
/**
 * 
 */
UCLASS()
class ARPG_API USkillObject : public UObject
{
	GENERATED_BODY()
public:
	void InitializeSkillObject(int32 _SkillID);

	void SetSkillLevel(int32 _SkillLevel) { SkillLevel = _SkillLevel; }
	void AddSkillLevel(int32 _AddLevel) { SkillLevel += _AddLevel; }
	void SetSkillID(int32 _SkillID) { SkillID = _SkillID; }
	void SetSkillVariantIDs(const TArray<int32>& _VariantIDs) { SkillVariantIDs = _VariantIDs; }
	void SetSkillVariantID(int32 _VariantID) { SkillVariantIDs.Add(_VariantID); }
	void RemoveSkillVariantID(int32 _VariantID) { SkillVariantIDs.Remove(_VariantID); }
	void SetVariantApplyIDs(int32 _AddVariantID);
	const TArray<int32>& GetSkillVariantIDs() const { return SkillVariantIDs; }
	const TSet<int32>& GetVariantApplyIDs() const;

	int32 GetSkillID() const { return SkillID; }
	const FGASSkillData* GetSkillData() const;
protected:
	int32 SkillID;
	int32 SkillLevel = 1;

	TArray<int32> SkillVariantIDs;
	TSet<int32> VariantApplyIDs;
};
