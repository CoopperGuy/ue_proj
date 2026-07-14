// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DiaRewardSubsystem.h"

#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "Engine/Texture2D.h"
#include "Logging/ARPGLogChannels.h"
#include "System/GASSkillManager.h"
#include "System/ItemSubsystem.h"
#include "System/JobSkillSetSubSystem.h"

#define LOCTEXT_NAMESPACE "DiaRewardSubsystem"

namespace
{
	constexpr int32 DefaultRoomClearGoldAmount = 100;
}

bool UDiaRewardSubsystem::TryMakeGoldReward(int32 GoldAmount, FRewardData& OutRewardData) const
{
	OutRewardData = FRewardData{};

	if (GoldAmount <= 0)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeGoldReward - GoldAmount must be positive: %d"), GoldAmount);
		return false;
	}

	OutRewardData.RewardType = ERewardType::Gold;
	OutRewardData.RewardRarity = ERewardRarity::Common;
	OutRewardData.DisplayName = FText::Format(LOCTEXT("GoldRewardName", "{0} Gold"), FText::AsNumber(GoldAmount));
	OutRewardData.Description = LOCTEXT("GoldRewardDescription", "Gain gold.");
	OutRewardData.GoldAmount = GoldAmount;

	return true;
}

bool UDiaRewardSubsystem::TryMakeItemReward(FName ItemId, int32 ItemLevel, int32 ItemQuantity, FRewardData& OutRewardData) const
{
	OutRewardData = FRewardData{};

	if (ItemId.IsNone())
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeItemReward - ItemId is none"));
		return false;
	}

	const UItemSubsystem* ItemSubsystem = GetItemSubsystem();
	if (!IsValid(ItemSubsystem))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeItemReward - ItemSubsystem is null"));
		return false;
	}

	const FItemBase* ItemData = ItemSubsystem->FindItemData(ItemId);
	if (!ItemData)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeItemReward - Item not found: %s"), *ItemId.ToString());
		return false;
	}

	OutRewardData.RewardType = ERewardType::Item;
	OutRewardData.RewardRarity = ConvertItemRarityToRewardRarity(ItemData->Rarity);
	OutRewardData.DisplayName = ItemData->Name;
	OutRewardData.Description = ItemData->Description;
	OutRewardData.Rarity = ItemData->Rarity;
	OutRewardData.ItemId = ItemData->ItemID;
	OutRewardData.ItemLevel = FMath::Max(1, ItemLevel);
	OutRewardData.ItemQuantity = FMath::Max(1, ItemQuantity);

	if (ItemData->IconPath.IsValid())
	{
		OutRewardData.Icon = Cast<UTexture2D>(ItemData->IconPath.TryLoad());
	}

	return true;
}

bool UDiaRewardSubsystem::TryMakeSkillAddReward(int32 SkillId, FRewardData& OutRewardData) const
{
	OutRewardData = FRewardData{};

	const UGASSkillManager* GASSkillManager = GetGASSkillManager();
	if (!IsValid(GASSkillManager))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeSkillAddReward - GASSkillManager is null"));
		return false;
	}

	const FGASSkillData* SkillData = GASSkillManager->GetSkillDataPtr(SkillId);
	if (!SkillData)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeSkillAddReward - Skill not found: %d"), SkillId);
		return false;
	}

	OutRewardData.RewardType = ERewardType::SkillAdd;
	OutRewardData.RewardRarity = ERewardRarity::Common;
	OutRewardData.DisplayName = SkillData->SkillName;
	OutRewardData.Description = SkillData->Description;
	OutRewardData.Icon = SkillData->Icon.LoadSynchronous();
	OutRewardData.SkillId = SkillData->SkillID;

	return true;
}

bool UDiaRewardSubsystem::TryMakeSkillUpgradeReward(int32 SkillId, FRewardData& OutRewardData) const
{
	OutRewardData = FRewardData{};

	const UGASSkillManager* GASSkillManager = GetGASSkillManager();
	if (!IsValid(GASSkillManager))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeSkillUpgradeReward - GASSkillManager is null"));
		return false;
	}

	const FGASSkillData* SkillData = GASSkillManager->GetSkillDataPtr(SkillId);
	if (!SkillData)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeSkillUpgradeReward - Skill not found: %d"), SkillId);
		return false;
	}

	OutRewardData.RewardType = ERewardType::SkillUpgrade;
	OutRewardData.RewardRarity = ERewardRarity::Rare;
	OutRewardData.DisplayName = SkillData->SkillName;
	OutRewardData.Description = SkillData->Description;
	OutRewardData.Icon = SkillData->Icon.LoadSynchronous();
	OutRewardData.SkillId = SkillData->SkillID;

	return true;
}

bool UDiaRewardSubsystem::TryMakeSkillVariantReward(int32 SkillId, int32 VariantId, FRewardData& OutRewardData) const
{
	OutRewardData = FRewardData{};

	const UGASSkillManager* GASSkillManager = GetGASSkillManager();
	if (!IsValid(GASSkillManager))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeSkillVariantReward - GASSkillManager is null"));
		return false;
	}

	const FGASSkillData* SkillData = GASSkillManager->GetSkillDataPtr(SkillId);
	if (!SkillData)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeSkillVariantReward - Skill not found: %d"), SkillId);
		return false;
	}

	if (!SkillData->VariantIDs.Contains(VariantId))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeSkillVariantReward - Variant %d is not registered on Skill %d"), VariantId, SkillId);
		return false;
	}

	const FSkillVariantData* VariantData = GASSkillManager->GetSkllVariantDataPtr(VariantId);
	if (!VariantData)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::TryMakeSkillVariantReward - Variant not found: %d"), VariantId);
		return false;
	}

	OutRewardData.RewardType = ERewardType::SkillVariant;
	OutRewardData.RewardRarity = ERewardRarity::Epic;
	OutRewardData.DisplayName = VariantData->VariantName;
	OutRewardData.Description = VariantData->Description;
	OutRewardData.Icon = VariantData->Icon.LoadSynchronous();
	if (!OutRewardData.Icon)
	{
		OutRewardData.Icon = SkillData->Icon.LoadSynchronous();
	}
	OutRewardData.SkillId = SkillData->SkillID;
	OutRewardData.VariantId = VariantData->VariantID;

	return true;
}

TArray<FRewardData> UDiaRewardSubsystem::MakeSkillAddRewardsForJob(EJobType JobType, int32 MaxRewardCount) const
{
	return MakeSkillAddRewardsForJob(JobType, nullptr, MaxRewardCount);
}

TArray<FRewardData> UDiaRewardSubsystem::MakeSkillAddRewardsForJob(EJobType JobType, const UDiaSkillManagerComponent* SkillManagerComponent, int32 MaxRewardCount) const
{
	TArray<FRewardData> Rewards;
	if (MaxRewardCount <= 0)
	{
		return Rewards;
	}
	if (IsValid(SkillManagerComponent) && !SkillManagerComponent->HasAvailableSkillSlot())
	{
		return Rewards;
	}

	const UJobSkillSetSubSystem* JobSkillSetSubsystem = GetJobSkillSetSubsystem();
	if (!IsValid(JobSkillSetSubsystem))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::MakeSkillAddRewardsForJob - JobSkillSetSubsystem is null"));
		return Rewards;
	}

	const FJobSkillSet& JobSkillSet = JobSkillSetSubsystem->GetJobSkillSet(JobType);
	for (const int32 SkillId : JobSkillSet.SkillIDs)
	{
		if (IsSkillRegistered(SkillManagerComponent, SkillId))
		{
			continue;
		}

		FRewardData RewardData;
		if (TryMakeSkillAddReward(SkillId, RewardData))
		{
			Rewards.Add(RewardData);
		}

		if (Rewards.Num() >= MaxRewardCount)
		{
			break;
		}
	}

	return Rewards;
}

TArray<FRewardData> UDiaRewardSubsystem::MakeSkillVariantRewardsForSkill(int32 SkillId, int32 MaxRewardCount) const
{
	return MakeSkillVariantRewardsForSkill(SkillId, nullptr, MaxRewardCount);
}

TArray<FRewardData> UDiaRewardSubsystem::MakeSkillVariantRewardsForSkill(int32 SkillId, const UDiaSkillManagerComponent* SkillManagerComponent, int32 MaxRewardCount) const
{
	TArray<FRewardData> Rewards;
	if (MaxRewardCount <= 0)
	{
		return Rewards;
	}

	if (IsValid(SkillManagerComponent) && !IsSkillRegistered(SkillManagerComponent, SkillId))
	{
		return Rewards;
	}

	const UGASSkillManager* GASSkillManager = GetGASSkillManager();
	if (!IsValid(GASSkillManager))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::MakeSkillVariantRewardsForSkill - GASSkillManager is null"));
		return Rewards;
	}

	const FGASSkillData* SkillData = GASSkillManager->GetSkillDataPtr(SkillId);
	if (!SkillData)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaRewardSubsystem::MakeSkillVariantRewardsForSkill - Skill not found: %d"), SkillId);
		return Rewards;
	}

	for (const int32 VariantId : SkillData->VariantIDs)
	{
		if (IsSkillVariantOwned(SkillManagerComponent, SkillId, VariantId))
		{
			continue;
		}

		FRewardData RewardData;
		if (TryMakeSkillVariantReward(SkillId, VariantId, RewardData))
		{
			Rewards.Add(RewardData);
		}

		if (Rewards.Num() >= MaxRewardCount)
		{
			break;
		}
	}

	return Rewards;
}

FRewardChoiceData UDiaRewardSubsystem::MakeRoomClearRewardChoice(EJobType JobType, int32 RewardCount) const
{
	return MakeRoomClearRewardChoice(JobType, nullptr, RewardCount);
}

FRewardChoiceData UDiaRewardSubsystem::MakeRoomClearRewardChoice(EJobType JobType, const UDiaSkillManagerComponent* SkillManagerComponent, int32 RewardCount) const
{
	FRewardChoiceData ChoiceData;
	ChoiceData.Title = LOCTEXT("RoomClearRewardTitle", "Room Clear Reward");
	ChoiceData.Subtitle = LOCTEXT("RoomClearRewardSubtitle", "Choose one reward.");

	const int32 TargetRewardCount = FMath::Max(0, RewardCount);
	if (TargetRewardCount == 0)
	{
		return ChoiceData;
	}

	FRewardData GoldReward;
	if (TryMakeGoldReward(DefaultRoomClearGoldAmount, GoldReward))
	{
		ChoiceData.RewardOptions.Add(GoldReward);
	}

	if (ChoiceData.RewardOptions.Num() < TargetRewardCount)
	{
		const TArray<FRewardData> SkillRewards = MakeSkillAddRewardsForJob(JobType, SkillManagerComponent, TargetRewardCount - ChoiceData.RewardOptions.Num());
		ChoiceData.RewardOptions.Append(SkillRewards);
	}

	if (ChoiceData.RewardOptions.Num() < TargetRewardCount)
	{
		const UJobSkillSetSubSystem* JobSkillSetSubsystem = GetJobSkillSetSubsystem();
		if (IsValid(JobSkillSetSubsystem))
		{
			const FJobSkillSet& JobSkillSet = JobSkillSetSubsystem->GetJobSkillSet(JobType);
			for (const int32 SkillId : JobSkillSet.SkillIDs)
			{
				const TArray<FRewardData> VariantRewards = MakeSkillVariantRewardsForSkill(SkillId, SkillManagerComponent, TargetRewardCount - ChoiceData.RewardOptions.Num());
				ChoiceData.RewardOptions.Append(VariantRewards);

				if (ChoiceData.RewardOptions.Num() >= TargetRewardCount)
				{
					break;
				}
			}
		}
	}

	if (ChoiceData.RewardOptions.Num() > TargetRewardCount)
	{
		ChoiceData.RewardOptions.SetNum(TargetRewardCount);
	}

	return ChoiceData;
}

UItemSubsystem* UDiaRewardSubsystem::GetItemSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UItemSubsystem>() : nullptr;
}

UGASSkillManager* UDiaRewardSubsystem::GetGASSkillManager() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UGASSkillManager>() : nullptr;
}

UJobSkillSetSubSystem* UDiaRewardSubsystem::GetJobSkillSetSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UJobSkillSetSubSystem>() : nullptr;
}

bool UDiaRewardSubsystem::IsSkillRegistered(const UDiaSkillManagerComponent* SkillManagerComponent, int32 SkillId) const
{
	return IsValid(SkillManagerComponent) && SkillManagerComponent->GetAbilitySpecBySkillID(SkillId) != nullptr;
}

bool UDiaRewardSubsystem::IsSkillVariantOwned(const UDiaSkillManagerComponent* SkillManagerComponent, int32 SkillId, int32 VariantId) const
{
	if (!IsValid(SkillManagerComponent))
	{
		return false;
	}

	const USkillObject* SkillObject = SkillManagerComponent->GetSkillObjectBySkillID(SkillId);
	return IsValid(SkillObject) && SkillObject->HasOwnedVariantID(VariantId);
}

ERewardRarity UDiaRewardSubsystem::ConvertItemRarityToRewardRarity(EItemRarity ItemRarity) const
{
	switch (ItemRarity)
	{
	case EItemRarity::EIR_Common:
	case EItemRarity::EIR_Uncommon:
		return ERewardRarity::Common;
	case EItemRarity::EIR_Rare:
		return ERewardRarity::Rare;
	case EItemRarity::EIR_Epic:
	case EItemRarity::EIR_Legendary:
	case EItemRarity::EIR_Mythic:
		return ERewardRarity::Epic;
	default:
		return ERewardRarity::None;
	}
}

#undef LOCTEXT_NAMESPACE
