// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DiaRewardSubsystem.h"
#include "Types/DiaRoomRewardPolicy.h"
#include "System/DiaMapGeneratorSubsystem.h"
#include "Map/DiaRoomBase.h"
#include "GAS/DiaAttributeSet.h"


#include "Controller/DiaController.h"
#include "DiaBaseCharacter.h"
#include "Character/DiaCharacter.h"

#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "DiaComponent/DiaOptionManagerComponent.h"

#include "DiaGameState.h"
#include "GAS/DiaGameplayTags.h"

#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Logging/ARPGLogChannels.h"

#include "System/GASSkillManager.h"
#include "System/ItemSubsystem.h"
#include "System/JobSkillSetSubSystem.h"

#define LOCTEXT_NAMESPACE "DiaRewardSubsystem"

namespace
{
	constexpr int32 DefaultRoomClearGoldAmount = 100;
}

namespace RewardBuild
{
	constexpr float EquippedSkillAffinityWeight = 2.0f;
	constexpr float OwnedSkillAffinityWeight = 0.5f;
	constexpr float SkillLevelAffinityBonusScale = 0.5f;

	struct FOptionAffinityRule
	{
		FGameplayTag BuildTag;
		float NormalizedValuePerPoint = 1.0f;
	};

	static bool TryGetOptionAffinityRule(
		const FGameplayTag& OptionTag,
		FOptionAffinityRule& OutRule)
	{
		const FDiaGameplayTags& Tags = FDiaGameplayTags::Get();

		if (OptionTag == Tags.ItemOptionStatCriticalChance)
		{
			OutRule = { Tags.Build_Stat_Crit, 0.05f };
			return true;
		}

		if (OptionTag == Tags.ItemOptionCooldownReduction)
		{
			OutRule = { Tags.Build_Stat_Cooldown, 0.05f };
			return true;
		}

		if (OptionTag == Tags.ItemOptionstatHealth)
		{
			OutRule = { Tags.Build_Stat_MaxHealth, 100.0f };
			return true;
		}

		if (OptionTag == Tags.ItemOptionStatDamage)
		{
			OutRule = { Tags.Build_Role_Damage, 0.10f };
			return true;
		}

		if (OptionTag == Tags.ItemOptionStatSpeed)
		{
			OutRule = { Tags.Build_Role_Mobility, 0.05f };
			return true;
		}

		if (OptionTag == Tags.ItemOptionLifeSteal)
		{
			OutRule = { Tags.Build_Role_Defense, 0.05f };
			return true;
		}

		// GoldFind는 전투 빌드 성향에 반영하지 않는다.
		return false;
	}

	static float CalculateOptionAffinityScore(
		const FDiaActualItemOption& Option,
		const FOptionAffinityRule& Rule)
	{
		float ComparableValue = 0.0f;

		switch (Option.ScalingType)
		{
		case EItemOptionScalingType::IOST_Flat:
			ComparableValue = Option.Value;
			break;

		case EItemOptionScalingType::IOST_Percent:
			ComparableValue = Option.Value * 0.01f;
			break;

		case EItemOptionScalingType::IOST_Multiplier:
			ComparableValue = Option.Value - 1.0f;
			break;

		default:
			return 0.0f;
		}

		if (Rule.NormalizedValuePerPoint <= UE_SMALL_NUMBER)
		{
			return 0.0f;
		}

		return FMath::Max(0.0f, ComparableValue) / Rule.NormalizedValuePerPoint;
	}

	static float CalculateSkillCandidateAffinity(
		const FGASSkillData& SkillData,
		const FRewardGenerationContext& Context)
	{
		float AffinityScore = 0.0f;
		const FGameplayTag& BuildRoot = FDiaGameplayTags::Get().Build_Root;

		for (const FGameplayTag& BuildTag : SkillData.AbilityTags.GetGameplayTagArray())
		{
			if (!BuildTag.IsValid() || !BuildTag.MatchesTag(BuildRoot))
			{
				continue;
			}

			if (const float* TagAffinity = Context.BuildAffinity.Find(BuildTag))
			{
				AffinityScore += *TagAffinity;
			}
		}

		return AffinityScore;
	}

	struct FDiaRewardContextBuilder
	{
	public:
		static FRewardGenerationContext Build(
			const ADiaController& Controller,
			const ADiaBaseCharacter& Character,
			const ADiaGameState& GameState)
		{
			FRewardGenerationContext OutContext;

			BuildSkillState(&Character, OutContext);
			BuildOptionState(Controller, OutContext);
			BuildProgressState(&GameState, OutContext);
			CalculateBuildAffinity(OutContext);

			return OutContext;
		}

	private:

		static void BuildSkillState(
			const ADiaBaseCharacter* Character,
			FRewardGenerationContext& OutContext)
		{
			const UDiaSkillManagerComponent* SkillComp = Character->GetSkillManagerComponent();
			if(SkillComp)
			{
				TArray<USkillObject*> SkillObjs = SkillComp->GetSkillIDMapping();
				for(const USkillObject* SkillObj : SkillObjs)
				{
					if(SkillObj)
					{
						OutContext.OwnedSkills.Emplace(
							SkillObj->GetSkillID(),
							SkillObj->GetSkillLevel(),
							SkillObj->GetSkillMaxLevel(),
							SkillComp->GetQuickSlotSkillIDs().Contains(SkillObj->GetSkillID()),
							SkillObj->GetSkillBuildTags(),
							SkillObj->GetOwnedVariantIDs());
					}
				}
			}
		}

		static void BuildOptionState(
			const ADiaController& Controller,
			FRewardGenerationContext& OutContext)
		{
			const UDiaOptionManagerComponent* OptionComp = Controller.GetOptionManagerComponent();
			if (!IsValid(OptionComp))
			{
				return;
			}

			const TMap<FName, FDiaActualItemOption>& Options = OptionComp->GetAllOptions();
			for (const TPair<FName, FDiaActualItemOption>& OptionPair : Options)
			{
				const FDiaActualItemOption& OptionData = OptionPair.Value;
				if (!OptionData.GrantedTag.IsValid())
				{
					continue;
				}

				FOptionAffinityRule Rule;
				if (!TryGetOptionAffinityRule(OptionData.GrantedTag, Rule))
				{
					continue;
				}

				const float Score = CalculateOptionAffinityScore(OptionData, Rule);
				if (Score <= 0.0f)
				{
					continue;
				}

				OutContext.OptionAffinity.FindOrAdd(Rule.BuildTag) += Score;
			}
		}

		static void BuildProgressState(
			const ADiaGameState* GameState,
			FRewardGenerationContext& OutContext)
		{

		}

		static void CalculateBuildAffinity(
			FRewardGenerationContext& OutContext)
		{
			OutContext.BuildAffinity.Reset();

			const FGameplayTag& BuildRoot = FDiaGameplayTags::Get().Build_Root;

			for (const TPair<FGameplayTag, float>& OptionAffinity : OutContext.OptionAffinity)
			{
				if (!OptionAffinity.Key.IsValid()
					|| !OptionAffinity.Key.MatchesTag(BuildRoot)
					|| OptionAffinity.Value <= 0.0f)
				{
					continue;
				}

				OutContext.BuildAffinity.FindOrAdd(OptionAffinity.Key) += OptionAffinity.Value;
			}

			for (const FRewardSkillState& SkillState : OutContext.OwnedSkills)
			{
				const float BaseWeight = SkillState.bEquipped
					? EquippedSkillAffinityWeight
					: OwnedSkillAffinityWeight;

				const float LevelRatio = SkillState.MaxLevel > 0
					? FMath::Clamp(
						static_cast<float>(SkillState.CurrentLevel) / static_cast<float>(SkillState.MaxLevel),
						0.0f,
						1.0f)
					: 0.0f;

				const float SkillScore = BaseWeight
					* (1.0f + LevelRatio * SkillLevelAffinityBonusScale);

				for (const FGameplayTag& BuildTag : SkillState.BuildTags.GetGameplayTagArray())
				{
					if (!BuildTag.IsValid() || !BuildTag.MatchesTag(BuildRoot))
					{
						continue;
					}

					OutContext.BuildAffinity.FindOrAdd(BuildTag) += SkillScore;
				}
			}

		}
	};
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
	return MakeSkillAddRewardsForJob(JobType, nullptr, nullptr, MaxRewardCount);
}

TArray<FRewardData> UDiaRewardSubsystem::MakeSkillAddRewardsForJob(EJobType JobType, const UDiaSkillManagerComponent* SkillManagerComponent, int32 MaxRewardCount) const
{
	return MakeSkillAddRewardsForJob(JobType, SkillManagerComponent, nullptr, MaxRewardCount);
}

TArray<FRewardData> UDiaRewardSubsystem::MakeSkillAddRewardsForJob(
	EJobType JobType,
	const UDiaSkillManagerComponent* SkillManagerComponent,
	const FRewardGenerationContext* RewardContext,
	int32 MaxRewardCount) const
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
	TArray<int32> CandidateSkillIds;
	for (const int32 SkillId : JobSkillSet.SkillIDs)
	{
		if (IsSkillRegistered(SkillManagerComponent, SkillId))
		{
			continue;
		}

		CandidateSkillIds.Add(SkillId);
	}

	const UGASSkillManager* GASSkillManager = GetGASSkillManager();
	if (RewardContext && IsValid(GASSkillManager))
	{
		CandidateSkillIds.StableSort(
			[GASSkillManager, RewardContext](const int32 LeftSkillId, const int32 RightSkillId)
			{
				const FGASSkillData* LeftSkillData = GASSkillManager->GetSkillDataPtr(LeftSkillId);
				const FGASSkillData* RightSkillData = GASSkillManager->GetSkillDataPtr(RightSkillId);

				const float LeftScore = LeftSkillData
					? RewardBuild::CalculateSkillCandidateAffinity(*LeftSkillData, *RewardContext)
					: 0.0f;
				const float RightScore = RightSkillData
					? RewardBuild::CalculateSkillCandidateAffinity(*RightSkillData, *RewardContext)
					: 0.0f;

				return LeftScore > RightScore;
			});
	}

	for (const int32 SkillId : CandidateSkillIds)
	{
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

	FRewardGenerationContext RewardContext;
	const FRewardGenerationContext* RewardContextPtr = nullptr;
	if (UWorld* World = GetWorld())
	{
		ADiaController* Controller = Cast<ADiaController>(World->GetFirstPlayerController());
		ADiaBaseCharacter* Character = Controller
			? Controller->GetPawn<ADiaBaseCharacter>()
			: nullptr;
		ADiaGameState* GameState = World->GetGameState<ADiaGameState>();

		if (IsValid(Controller) && IsValid(Character) && IsValid(GameState))
		{
			RewardContext = RewardBuild::FDiaRewardContextBuilder::Build(
				*Controller,
				*Character,
				*GameState);
			RewardContextPtr = &RewardContext;
		}
	}

	FRewardData GoldReward;
	if (TryMakeGoldReward(DefaultRoomClearGoldAmount, GoldReward))
	{
		ChoiceData.RewardOptions.Add(GoldReward);
	}

	if (ChoiceData.RewardOptions.Num() < TargetRewardCount)
	{
		const TArray<FRewardData> SkillRewards = MakeSkillAddRewardsForJob(
			JobType,
			SkillManagerComponent,
			RewardContextPtr,
			TargetRewardCount - ChoiceData.RewardOptions.Num());
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


const FDiaRoomRewardState* UDiaRewardSubsystem::ResolveRoomReward(
    const FGuid& Room, EDiaRoomRewardKind Kind, ADiaController* Controller)
{
    if (const FDiaRoomRewardState* Existing = RoomRewards.Find(Room); Existing && Existing->bResolved)
    {
        return Existing;
    }
    ADiaBaseCharacter* Character = IsValid(Controller) ? Cast<ADiaBaseCharacter>(Controller->GetPawn()) : nullptr;
    UDiaSkillManagerComponent* Skills = Character ? Character->FindComponentByClass<UDiaSkillManagerComponent>() : nullptr;
    UItemSubsystem* Items = GetItemSubsystem();
    if (!Room.IsValid() || !Character || !Skills || !Items) return nullptr;

    FRandomStream Random(static_cast<int32>(GetTypeHash(Room) ^ 0x6C8E9CF5u));
    FDiaRoomRewardState State;
    State.Kind = Kind;
    const int32 Level = Character->GetAttributeSet() ? FMath::Max(1, FMath::RoundToInt(Character->GetAttributeSet()->GetLevel())) : 1;
    FRewardData Reward;
    if (Kind == EDiaRoomRewardKind::Normal)
    {
        switch (DiaRoomRewardPolicy::FromPercentile(Random.RandRange(0, 99)))
        {
        case DiaRoomRewardPolicy::EDrop::None: break;
        case DiaRoomRewardPolicy::EDrop::Gold:
            if (TryMakeGoldReward(20 + 5 * Level, Reward)) State.Options.Add(Reward);
            break;
        case DiaRoomRewardPolicy::EDrop::Heal:
            Reward.RewardType = ERewardType::Heal;
            Reward.DisplayName = LOCTEXT("RoomHeal", "회복 구슬");
            Reward.Description = LOCTEXT("RoomHealDesc", "최대 체력의 25%를 회복합니다. 체력이 가득 차면 남겨 둡니다.");
            State.Options.Add(Reward);
            break;
        case DiaRoomRewardPolicy::EDrop::Equipment:
        {
            const TArray<FName> IDs = Items->GetRewardEquipmentIDs(true);
            if (IDs.IsEmpty() || !TryMakeItemReward(IDs[Random.RandRange(0, IDs.Num() - 1)], Level, 1, Reward))
            {
                UE_LOG(LogARPG, Error, TEXT("Room reward requires at least one common equippable item."));
                return nullptr;
            }
            State.Options.Add(Reward);
            break;
        }
        }
    }
    else
    {
        // New skills retain the existing build-affinity ordering. Shortlist, then
        // sample categories so a large equipment table cannot drown out skills.
        FRewardGenerationContext Context;
        if (const ADiaGameState* GS = Controller->GetWorld()->GetGameState<ADiaGameState>())
            Context = RewardBuild::FDiaRewardContextBuilder::Build(*Controller, *Character, *GS);
        const ADiaCharacter* Player = Cast<ADiaCharacter>(Character);
        const EJobType Job = Player ? Player->GetJobType() : EJobType::Warrior;
        int32 ReservedSkillSlots = 0;
        for (const auto& Entry : RoomRewards)
        {
            if (!Entry.Value.bClaimed && Entry.Value.Options.ContainsByPredicate([](const FRewardData& Pending)
                { return Pending.RewardType == ERewardType::SkillAdd; })) ++ReservedSkillSlots;
        }
        TArray<FRewardData> SkillPool;
        if (Skills->GetSkillIDMapping().Num() + ReservedSkillSlots < UDiaSkillManagerComponent::MaxOwnedSkillCount)
            SkillPool = MakeSkillAddRewardsForJob(Job, Skills, &Context, 3);
        for (const USkillObject* Skill : Skills->GetSkillIDMapping())
        {
            if (!IsValid(Skill)) continue;
            SkillPool.Append(MakeSkillVariantRewardsForSkill(Skill->GetSkillID(), Skills, 32));
            if (Kind == EDiaRoomRewardKind::Boss && Skill->GetSkillLevel() < Skill->GetSkillMaxLevel()
                && TryMakeSkillUpgradeReward(Skill->GetSkillID(), Reward)) SkillPool.Add(Reward);
        }
        SkillPool.RemoveAll([this](const FRewardData& Candidate)
        {
            for (const auto& Entry : RoomRewards)
            {
                if (Entry.Value.bClaimed) continue;
                for (const FRewardData& Pending : Entry.Value.Options)
                    if (Pending.RewardType == Candidate.RewardType && Pending.SkillId == Candidate.SkillId
                        && Pending.VariantId == Candidate.VariantId) return true;
            }
            return false;
        });
        TArray<FRewardData> EquipmentPool;
        for (const FName ID : Items->GetRewardEquipmentIDs())
        {
            FRewardData Equipment;
            Equipment.RewardType = ERewardType::Item;
            Equipment.ItemId = ID;
            Equipment.ItemLevel = Level;
            EquipmentPool.Add(Equipment);
        }
        const int32 Count = Kind == EDiaRoomRewardKind::Boss ? 2 : 1;
        for (int32 Index = 0; Index < Count; ++Index)
        {
            const bool bUseSkill = !SkillPool.IsEmpty() && (EquipmentPool.IsEmpty() || Random.FRand() < 0.75f);
            TArray<FRewardData>& Pool = bUseSkill ? SkillPool : EquipmentPool;
            if (Pool.IsEmpty()) break;
            const int32 Pick = Random.RandRange(0, Pool.Num() - 1);
            State.Options.Add(Pool[Pick]);
            Pool.RemoveAt(Pick);
        }
        if (State.Options.Num() != Count)
        {
            UE_LOG(LogARPG, Error, TEXT("Room %s has insufficient valid growth rewards (%d/%d)."), *Room.ToString(), State.Options.Num(), Count);
            return nullptr; // Remain unresolved and visible; never silently consume a guaranteed reward.
        }
    }

    for (int32 Index = 0; Index < State.Options.Num(); ++Index)
    {
        FRewardData& Option = State.Options[Index];
        if (Option.RewardType == ERewardType::Item)
        {
            FRewardData Detail;
            if (!TryMakeItemReward(Option.ItemId, Option.ItemLevel, Option.ItemQuantity, Detail)) return nullptr;
            Option = MoveTemp(Detail);
            Items->CreateInventoryInstance(Option.PreparedItem, Option.ItemId, Option.ItemLevel, true, Option.ItemQuantity);
            if (Option.PreparedItem.IsEmpty()) return nullptr;
            Option.bHasPreparedItem = true;
            FString Description = Option.Description.ToString();
            for (const FDiaActualItemOption& Affix : Option.PreparedItem.ItemInstance.PrefixOptions)
                Description += TEXT("\n") + Affix.GetOptionDescription().ToString();
            for (const FDiaActualItemOption& Affix : Option.PreparedItem.ItemInstance.SuffixOptions)
                Description += TEXT("\n") + Affix.GetOptionDescription().ToString();
            Option.Description = FText::FromString(Description);
        }
        Option.SourceRoomGuid = Room;
        Option.SourceOptionIndex = Index;
    }
    State.bResolved = true;
    RoomRewards.Add(Room, MoveTemp(State));
    return RoomRewards.Find(Room);
}

bool UDiaRewardSubsystem::TryClaimRoomReward(const FGuid& Room, int32 OptionIndex, ADiaController* Controller)
{
    FDiaRoomRewardState* State = RoomRewards.Find(Room);
    if (bClaimInProgress || !State || !State->bResolved || State->bClaimed || !State->Options.IsValidIndex(OptionIndex)
        || !IsValid(Controller) || !Controller->GetPawn()) return false;
    const ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(Controller->GetPawn());
    if (!Character || !Character->GetAttributeSet() || Character->GetAttributeSet()->GetHealth() <= 0.f) return false;
    UWorld* World = Controller->GetWorld();
    const ADiaGameState* GS = World->GetGameState<ADiaGameState>();
    const UDiaMapGeneratorSubsystem* Map = World->GetSubsystem<UDiaMapGeneratorSubsystem>();
    const ADiaRoomBase* RoomActor = Map ? Map->GetRoomActor(Room).Get() : nullptr;
    if (!GS || GS->IsInBattle() || !RoomActor
        || (RoomActor->GetRoomRole() != EDiaRoomRole::Treasure && !GS->IsRoomCleared(Room))
        || FVector::DistSquared2D(Controller->GetPawn()->GetActorLocation(), RoomActor->GetActorLocation()) > FMath::Square(350.f)) return false;

    TGuardValue<bool> Guard(bClaimInProgress, true);
    const FRewardData Selected = State->Options[OptionIndex];
    if (!Controller->ApplyReward(Selected)) return false;
    // ApplyReward may broadcast delegates; reacquire the map entry afterwards.
    RoomRewards.FindChecked(Room).bClaimed = true;
    return true;
}

#undef LOCTEXT_NAMESPACE
