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
};

