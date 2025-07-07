// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DiaCharacterTable.generated.h"

// 캐릭터 클래스/직업 타입
UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	Warrior		UMETA(DisplayName = "전사"),
	Mage		UMETA(DisplayName = "마법사"),
	Archer		UMETA(DisplayName = "궁수"),
	Rogue		UMETA(DisplayName = "도적"),
	None		UMETA(DisplayName = "없음")
};

// 플레이어 캐릭터 기본 정보 구조체
USTRUCT(BlueprintType)
struct ARPG_API FCharacterInfo : public FTableRowBase
{
	GENERATED_BODY()

	// 캐릭터 식별자
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName CharacterID;

	// 캐릭터 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText Name;

	// 캐릭터 클래스/직업
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	// 기본 레벨
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	int32 StartLevel = 1;

	// 체력 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float BaseMaxHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float HPPerLevel = 10.0f;

	// 마나 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana")
	float BaseMaxMP = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mana")
	float MPPerLevel = 5.0f;

	// 기본 스탯 (Str, Int, Dex, Con)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseStats")
	float BaseStrength = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseStats")
	float BaseIntelligence = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseStats")
	float BaseDexterity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaseStats")
	float BaseConstitution = 10.0f;

	// 레벨당 스탯 증가량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatGrowth")
	float StrengthPerLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatGrowth")
	float IntelligencePerLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatGrowth")
	float DexterityPerLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatGrowth")
	float ConstitutionPerLevel = 1.0f;

	// 전투 스탯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseAttackPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseDefense = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseAttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float BaseAttackRange = 150.0f;

	// 전투 스탯 레벨당 증가량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatGrowth")
	float AttackPowerPerLevel = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatGrowth")
	float DefensePerLevel = 1.0f;

	// 경험치 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
	float BaseExpRequired = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Experience")
	float ExpGrowthRate = 1.5f; // 레벨당 경험치 증가 배율

	// 초기 보유 스킬 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skills")
	TArray<int32> InitialSkillIDs;

	// 캐릭터 메시 (외형)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TSoftObjectPtr<USkeletalMesh> CharacterMesh;

	// 애니메이션 블루프린트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> AnimationBlueprint;
};

// 캐릭터 생성 정보 (게임 시작시 사용)
USTRUCT(BlueprintType)
struct ARPG_API FCharacterCreationInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CharacterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterClass SelectedClass = ECharacterClass::Warrior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartLevel = 1;
}; 