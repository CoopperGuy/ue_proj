// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/DiaGASSkillData.h"
#include "Types/DiaRewardData.h"
#include "Types/DiaRoomRewardState.h"
#include "DiaRewardSubsystem.generated.h"

class ADiaController;
class UItemSubsystem;
class UGASSkillManager;
class UJobSkillSetSubSystem;
class UDiaSkillManagerComponent;

/**
 * 
 */
UCLASS()
class ARPG_API UDiaRewardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    bool IsApplyingReward() const { return bClaimInProgress; }
    void ResetFloor() { RoomRewards.Reset(); }
    void RestoreFloor(const TMap<FGuid, FDiaRoomRewardState>& Saved) { RoomRewards = Saved; }
    const TMap<FGuid, FDiaRoomRewardState>& GetRoomRewards() const { return RoomRewards; }
    const FDiaRoomRewardState* FindRoomReward(const FGuid& Room) const { return RoomRewards.Find(Room); }
    const FDiaRoomRewardState* ResolveRoomReward(const FGuid& Room, EDiaRoomRewardKind Kind, ADiaController* Controller);
    bool TryClaimRoomReward(const FGuid& Room, int32 OptionIndex, ADiaController* Controller);

	bool TryMakeGoldReward(int32 GoldAmount, FRewardData& OutRewardData) const;
	bool TryMakeItemReward(FName ItemId, int32 ItemLevel, int32 ItemQuantity, FRewardData& OutRewardData) const;
	bool TryMakeSkillAddReward(int32 SkillId, FRewardData& OutRewardData) const;
	bool TryMakeSkillUpgradeReward(int32 SkillId, FRewardData& OutRewardData) const;
	bool TryMakeSkillVariantReward(int32 SkillId, int32 VariantId, FRewardData& OutRewardData) const;
	TArray<FRewardData> MakeSkillAddRewardsForJob(EJobType JobType, int32 MaxRewardCount = 3) const;
	TArray<FRewardData> MakeSkillAddRewardsForJob(EJobType JobType, const UDiaSkillManagerComponent* SkillManagerComponent, int32 MaxRewardCount = 3) const;
	TArray<FRewardData> MakeSkillVariantRewardsForSkill(int32 SkillId, int32 MaxRewardCount = 3) const;
	TArray<FRewardData> MakeSkillVariantRewardsForSkill(int32 SkillId, const UDiaSkillManagerComponent* SkillManagerComponent, int32 MaxRewardCount = 3) const;
	FRewardChoiceData MakeRoomClearRewardChoice(EJobType JobType, int32 RewardCount = 3) const;
	FRewardChoiceData MakeRoomClearRewardChoice(EJobType JobType, const UDiaSkillManagerComponent* SkillManagerComponent, int32 RewardCount = 3) const;

private:
    UPROPERTY() TMap<FGuid, FDiaRoomRewardState> RoomRewards;
    bool bClaimInProgress = false;

	TArray<FRewardData> MakeSkillAddRewardsForJob(
		EJobType JobType,
		const UDiaSkillManagerComponent* SkillManagerComponent,
		const FRewardGenerationContext* RewardContext,
		int32 MaxRewardCount) const;

	UItemSubsystem* GetItemSubsystem() const;
	UGASSkillManager* GetGASSkillManager() const;
	UJobSkillSetSubSystem* GetJobSkillSetSubsystem() const;

	bool IsSkillRegistered(const UDiaSkillManagerComponent* SkillManagerComponent, int32 SkillId) const;
	bool IsSkillVariantOwned(const UDiaSkillManagerComponent* SkillManagerComponent, int32 SkillId, int32 VariantId) const;
	ERewardRarity ConvertItemRarityToRewardRarity(EItemRarity ItemRarity) const;
};
