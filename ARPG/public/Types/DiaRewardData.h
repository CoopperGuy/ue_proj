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
	SkillVariant UMETA(DisplayName = "스킬 강화")
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
	int32 SkillId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VariantId = 0;
};
