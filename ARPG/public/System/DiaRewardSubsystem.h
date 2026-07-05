// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/DiaGASSkillData.h"
#include "Types/DiaRewardData.h"
#include "DiaRewardSubsystem.generated.h"

class UItemSubsystem;
class UGASSkillManager;
class UJobSkillSetSubSystem;

/**
 * 
 */
UCLASS()
class ARPG_API UDiaRewardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	bool TryMakeGoldReward(int32 GoldAmount, FRewardData& OutRewardData) const;
	bool TryMakeItemReward(FName ItemId, int32 ItemLevel, int32 ItemQuantity, FRewardData& OutRewardData) const;
	bool TryMakeSkillAddReward(int32 SkillId, FRewardData& OutRewardData) const;
	bool TryMakeSkillUpgradeReward(int32 SkillId, FRewardData& OutRewardData) const;
	bool TryMakeSkillVariantReward(int32 SkillId, int32 VariantId, FRewardData& OutRewardData) const;
	TArray<FRewardData> MakeSkillAddRewardsForJob(EJobType JobType, int32 MaxRewardCount = 3) const;
	TArray<FRewardData> MakeSkillVariantRewardsForSkill(int32 SkillId, int32 MaxRewardCount = 3) const;
	FRewardChoiceData MakeRoomClearRewardChoice(EJobType JobType, int32 RewardCount = 3) const;

private:
	UItemSubsystem* GetItemSubsystem() const;
	UGASSkillManager* GetGASSkillManager() const;
	UJobSkillSetSubSystem* GetJobSkillSetSubsystem() const;

	ERewardRarity ConvertItemRarityToRewardRarity(EItemRarity ItemRarity) const;
};
