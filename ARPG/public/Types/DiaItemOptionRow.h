// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DiaItemOptionRow.generated.h"

UENUM(BlueprintType)
enum class EItemOptionType : uint8
{
	IOT_Prefix      UMETA(DisplayName = "Prefix"),
	IOT_Suffix      UMETA(DisplayName = "Suffix"),
};

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	EES_None UMETA(DisplayName = "None"),
	EES_Head UMETA(DisplayName = "Head"),
	EES_Chest UMETA(DisplayName = "Chest"),
	EES_Legs UMETA(DisplayName = "Legs"),
	EES_Feet UMETA(DisplayName = "Feet"),
	EES_Hands UMETA(DisplayName = "Hands"),
	EES_Weapon UMETA(DisplayName = "Weapon"),
	EES_Shield UMETA(DisplayName = "Shield"),
	EES_RingL UMETA(DisplayName = "RingL"),
	EES_RingR UMETA(DisplayName = "RingR"),
	EES_Amulet UMETA(DisplayName = "Amulet"),
	EES_Max UMETA(DisplayName = "Max")
};

UENUM(BlueprintType)
enum class EItemOptionScalingType : uint8
{
	IOST_Flat        UMETA(DisplayName = "Flat"),       
	IOST_Percent     UMETA(DisplayName = "Percent"),    
	IOST_Multiplier  UMETA(DisplayName = "Multiplier"), 
};


USTRUCT(BlueprintType)
struct ARPG_API FDiaItemOptionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName OptionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TierIndex = 1;

	/// <summary>
	/// TierIndex 와 OptionID 를 혼합해 UniqueKey 생성 가능
	/// TierIndex 가 낮을수록 강력한 옵션
	/// </summary>

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemOptionType OptionType = EItemOptionType::IOT_Prefix;

	//옵션 붙을 수 있는 슬롯
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<EEquipmentSlot> SlotTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredItemLevelMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredItemLevelMax = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemOptionScalingType ScalingType = EItemOptionScalingType::IOST_Flat;

	// 등장 확률 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Weight = 100;

	// 이 옵션을 사용할 수 있는 조건 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer RequiredTags;

	// 이 옵션을 금지하는 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTagContainer BlockedTags;

	//실제 효과 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag GrantedTag;

	FName GetUniqueOptionKey() const
	{
		return FName(*FString::Printf(TEXT("%s_Tier%d"), *OptionID.ToString(), TierIndex));
	}

	double MakeRandomValue() const
	{
		double Z = 0;
		for (int32 i = 0; i < 12; ++i)
		{
			Z += FMath::FRand();
		}
		Z -= 6.0;

		double MidValue = (MinValue + MaxValue) * 0.5f;
		double HalfRange = (MaxValue - MinValue) * 0.5f;

		//12개 합의 분산은 1/12 이므로 표준편차는 sqrt(1/12) = 0.288675
		return MidValue + HalfRange * Z * 0.1667f; 
	}
};

USTRUCT(BlueprintType)
struct ARPG_API FDiaActualItemOption
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName OptionID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TierIndex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Value = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemOptionType OptionType = EItemOptionType::IOT_Prefix;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemOptionScalingType ScalingType = EItemOptionScalingType::IOST_Flat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag GrantedTag;

	FName GetUniqueOptionKey() const
	{
		return FName(*FString::Printf(TEXT("%s_Tier%d"), *OptionID.ToString(), TierIndex));
	}

	FDiaActualItemOption()
	{
		
	}
	FDiaActualItemOption(const FDiaItemOptionRow& OptionRow)
		: OptionID(OptionRow.OptionID)
		, TierIndex(OptionRow.TierIndex)
		, Value(OptionRow.MakeRandomValue())
		, DisplayName(OptionRow.DisplayName)
		, OptionType(OptionRow.OptionType)
		, ScalingType(OptionRow.ScalingType)
		, GrantedTag(OptionRow.GrantedTag)
	{
	}

	FText GetOptionDescription() const
	{
		FString ValueStr;
		switch (ScalingType)
		{
		case EItemOptionScalingType::IOST_Flat:
			ValueStr = FString::Printf(TEXT("%.0f"), Value);
			break;
		case EItemOptionScalingType::IOST_Percent:
			ValueStr = FString::Printf(TEXT("%.1f%%"), Value * 100.f);
			break;
		case EItemOptionScalingType::IOST_Multiplier:
			ValueStr = FString::Printf(TEXT("x%.2f"), Value);
			break;
		default:
			ValueStr = FString::Printf(TEXT("%.0f"), Value);
			break;
		}
		return FText::FromString(FString::Printf(TEXT("%s %s"), *DisplayName.ToString(), *ValueStr));
	}
};

