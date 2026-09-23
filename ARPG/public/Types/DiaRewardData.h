#pragma once

#include "CoreMinimal.h"
#include "Types/ItemBase.h"
#include "DiaRewardData.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class ERewardType : uint8
{
	None,
	Gold UMETA(DisplayName = "골드"),
	Item UMETA(DisplayName = "아이템"),
	SkillAdd UMETA(DisplayName = "스킬 추가"),
	SkillUpgrade UMETA(DisplayName = "스킬 레벨 업"),
	SkillVariant UMETA(DisplayName = "스킬 강화"),
	Heal UMETA(DisplayName = "회복")
};

UENUM(BlueprintType)
enum class ERewardRarity : uint8
{
	None,
	Common UMETA(DisplayName = "일반"),
	Rare UMETA(DisplayName = "희귀"),
	Epic UMETA(DisplayName = "영웅"),
};

USTRUCT(BlueprintType)
struct ARPG_API FRewardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERewardType RewardType = ERewardType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERewardRarity RewardRarity = ERewardRarity::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemRarity Rarity = EItemRarity::EIR_Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VariantId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HealFraction = 0.25f;

    // Identity references the authoritative room ledger, never a widget copy.
    UPROPERTY()
    FGuid SourceRoomGuid;
    UPROPERTY()
    int32 SourceOptionIndex = INDEX_NONE;
    UPROPERTY()
    bool bHasPreparedItem = false;
    UPROPERTY()
    FInventorySlot PreparedItem;
};

USTRUCT(BlueprintType)
struct ARPG_API FRewardChoiceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Subtitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRewardData> RewardOptions;
};

USTRUCT()
struct FRewardSkillState
{
	GENERATED_BODY()

	int32 SkillID = 0;

	int32 CurrentLevel = 0;
	int32 MaxLevel = 0;

	bool bEquipped = false;

	FGameplayTagContainer BuildTags;

	TSet<int32> OwnedVariants;

	FRewardSkillState() = default;

	FRewardSkillState(
		int32 InSkillID,
		int32 InCurrentLevel,
		int32 InMaxLevel,
		bool bInEquipped,
		const FGameplayTagContainer& InBuildTags,
		const TSet<int32>& InOwnedVariants)
		: SkillID(InSkillID)
		, CurrentLevel(InCurrentLevel)
		, MaxLevel(InMaxLevel)
		, bEquipped(bInEquipped)
		, BuildTags(InBuildTags)
		, OwnedVariants(InOwnedVariants)
	{
	}
};

USTRUCT(BlueprintType)
struct ARPG_API FRewardGenerationContext
{
	GENERATED_BODY()

	// Character
	int32 CharacterLevel = 1;

	// Skill
	int32 AvailableSkillSlots = 0;
	TArray<FRewardSkillState> OwnedSkills;

	// Build
	TMap<FGameplayTag, float> OptionAffinity;
	TMap<FGameplayTag, float> BuildAffinity;

	// Progress
	int32 ClearedRoomCount = 0;

	// Anti-repeat
	//TArray<FRewardKey> RecentOfferedRewards;
};
