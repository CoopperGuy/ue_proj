// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/DiaMonsterTable.h"
#include "Types/DiaCharacterTable.h"
#include "Types/ItemBase.h"
#include "DiaStatComponent.generated.h"

class UCharacterManager;

// 캐릭터 기본 스탯 구조체 (체력, 마나만 관리)
USTRUCT(BlueprintType)
struct ARPG_API FCharacterData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth_Additional = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Mana = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMana = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMana_Additional = 0.f;

	// 기본 스탯 배열 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> DefStats; 

	// 추가 스탯 배열
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> AdditinalStats; 

	// 스탯 접근 함수들 (타입 안전성 제공)
	FORCEINLINE float GetStat(EItemStat StatType) const
	{
		int32 Index = ToInt(StatType);
		return DefStats.IsValidIndex(Index) ? DefStats[Index] : 0.0f;
	}

	FORCEINLINE void SetStat(EItemStat StatType, float Value)
	{
		int32 Index = ToInt(StatType);
		if (DefStats.IsValidIndex(Index))
		{
			DefStats[Index] = Value;
		}
	}

	FORCEINLINE float GetAdditionalStat(EItemStat StatType) const
	{
		int32 Index = ToInt(StatType);
		return AdditinalStats.IsValidIndex(Index) ? AdditinalStats[Index] : 0.0f;
	}

	FORCEINLINE void SetAdditionalStat(EItemStat StatType, float Value)
	{
		int32 Index = ToInt(StatType);
		if (AdditinalStats.IsValidIndex(Index))
		{
			AdditinalStats[Index] = Value;
		}
	}

	// 편의를 위한 개별 스탯 접근 함수들
	FORCEINLINE float GetStrength() const { return GetStat(EItemStat::EIS_Str); }
	FORCEINLINE float GetIntelligence() const { return GetStat(EItemStat::EIS_Int); }
	FORCEINLINE float GetDexterity() const { return GetStat(EItemStat::EIS_Dex); }
	FORCEINLINE float GetConstitution() const { return GetStat(EItemStat::EIS_Con); }

	FORCEINLINE void SetStrength(float Value) { SetStat(EItemStat::EIS_Str, Value); }
	FORCEINLINE void SetIntelligence(float Value) { SetStat(EItemStat::EIS_Int, Value); }
	FORCEINLINE void SetDexterity(float Value) { SetStat(EItemStat::EIS_Dex, Value); }
	FORCEINLINE void SetConstitution(float Value) { SetStat(EItemStat::EIS_Con, Value); }

	void InitializeStatArrays()
	{
		for(int32 i = 0; i < STAT_ARRAY_SIZE; ++i)
		{
			DefStats.Add(0.0f);
			AdditinalStats.Add(0.0f);
		}
	}

	// 몬스터에게서도 해당 스탯 구조를 사용하기 위해 만든 것
	FCharacterData& operator=(const FMonsterInfo& Other)
	{
		Health = Other.MaxHP;
		MaxHealth = Other.MaxHP;
		Mana = Other.MaxMP;
		MaxMana = Other.MaxMP;
		return *this;
	}
};

// 전투 관련 스탯 구조체
USTRUCT(BlueprintType)
struct ARPG_API FCombatStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackPower = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRange = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Defense = 100.f;

	FCombatStats& operator=(const FMonsterInfo& Other)
	{
		AttackPower = Other.Attack;
		AttackSpeed = Other.AttackSpeed;
		AttackRange = Other.AttackRange;
		Defense = Other.Defense;
		return *this;
	}
};

// 레벨 및 경험치 관리 구조체
USTRUCT(BlueprintType)
struct ARPG_API FLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentExp = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxExp = 100.f;

	// 레벨별 경험치 요구량 계산
	float GetRequiredExpForLevel(int32 Level) const
	{
		return 100.0f + (Level - 1) * 50.0f; // 레벨당 50씩 증가
	}
};

//// 스탯 변경 델리게이트
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, NewHealth, float, MaxHealth);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChangedDelegate, float, NewMana, float, MaxMana);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExpChangedDelegate, float, NewExp, float, MaxExp);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpDelegate, int32, NewLevel);
//
//// 기본 스탯 변경 델리게이트 (통합)
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBaseStatChangedDelegate, EItemStat, StatType, float, NewValue, float, OldValue);
//
//// 전투 스탯 변경 델리게이트
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackPowerChangedDelegate, float, NewAttackPower, float, OldAttackPower);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDefenseChangedDelegate, float, NewDefense, float, OldDefense);
//
//// 초기화 완료 델리게이트 (UI 바인딩 타이밍용)
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatComponentInitializedDelegate, UDiaStatComponent*, StatComponent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UDiaStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDiaStatComponent();

//	// 초기화 (몬스터용)
//	void InitializeFromData(const FMonsterInfo& MonsterInfo);
//
//	// 초기화 (플레이어용)
//	void InitializeFromCharacterData(FName CharacterID, int32 Level = 1);
//
//	// 체력 관리
//	void TakeDamage(float DamageAmount);
//	void RestoreHealth(float HealthAmount);
//	void SetMaxHealth(float NewMaxHealth);
//
//	// 마나 관리
//	void ConsumeMana(float ManaAmount);
//	void RestoreMana(float ManaAmount);
//	void SetMaxMana(float NewMaxMana);
//	bool HasEnoughMana(float ManaAmount) const;
//
//	// 레벨 및 경험치 관리
//	void AddExperience(float ExpAmount);
//	void SetLevel(int32 NewLevel);
//	bool CanLevelUp() const;
//
//	// 스탯 검증
//	bool IsDead() const { return CharacterData.Health <= 0.0f; }
//	bool IsAlive() const { return CharacterData.Health > 0.0f; }
//	
//	// 초기화 상태 확인
//	bool IsInitialized() const { return bIsInitialized; }
//
//protected:
//	virtual void BeginPlay() override;
//	
//	// 레벨업 처리 내부 함수
//	void ProcessLevelUpStatBonus();
//	void RecalculateStatsForLevel(int32 Level);
//
//public:
//	//////////////////////////////////////////////////////////////////////////
//	// Getter 함수들
//	//////////////////////////////////////////////////////////////////////////
//	
//	// 기본 스탯 정보
//	const FCharacterData& GetCharacterData() const { return CharacterData; }
//	void SetCharacterData(const FCharacterData& NewData) { CharacterData = NewData; }
//	
//	FORCEINLINE float GetCurrentHealth() const { return CharacterData.Health; }
//	FORCEINLINE float GetMaxHealth() const { return CharacterData.MaxHealth; }
//	FORCEINLINE float GetCurrentMana() const { return CharacterData.Mana; }
//	FORCEINLINE float GetMaxMana() const { return CharacterData.MaxMana; }
//	FORCEINLINE float GetHealthPercentage() const { return CharacterData.Health / CharacterData.MaxHealth; }
//	FORCEINLINE float GetManaPercentage() const { return CharacterData.Mana / CharacterData.MaxMana; } 
//
//	// 레벨 관련 Getter
//	const FLevelData& GetLevelData() const { return LevelData; }
//	FORCEINLINE int32 GetCurrentLevel() const { return LevelData.CurrentLevel; }
//	FORCEINLINE float GetCurrentExp() const { return LevelData.CurrentExp; }
//	FORCEINLINE float GetMaxExp() const { return LevelData.MaxExp; }
//	FORCEINLINE float GetExpPercentage() const { return LevelData.CurrentExp / LevelData.MaxExp; }
//
//	// 전투 스탯 정보
//	const FCombatStats& GetCombatStats() const { return CombatStats; }
//	void SetCombatStats(const FCombatStats& NewStats) { CombatStats = NewStats; }
//	
//	FORCEINLINE float GetAttackPower() const { return CombatStats.AttackPower; }
//	FORCEINLINE float GetAttackSpeed() const { return CombatStats.AttackSpeed; }
//	FORCEINLINE float GetAttackRange() const { return CombatStats.AttackRange; }
//	FORCEINLINE float GetDefense() const { return CombatStats.Defense; }
//
//	// 스탯 변경 (델리게이트 포함)
//	void SetAttackPower(float NewAttackPower);
//	void SetAttackSpeed(float NewAttackSpeed) { CombatStats.AttackSpeed = NewAttackSpeed; }
//	void SetAttackRange(float NewAttackRange) { CombatStats.AttackRange = NewAttackRange; }
//	void SetDefense(float NewDefense);
//
//	// 기본 스탯 변경 함수들 (델리게이트 포함)
//	void SetBaseStat(EItemStat StatType, float NewValue);
//	void SetStrength(float NewValue) { SetBaseStat(EItemStat::EIS_Str, NewValue); }
//	void SetIntelligence(float NewValue) { SetBaseStat(EItemStat::EIS_Int, NewValue); }
//	void SetDexterity(float NewValue) { SetBaseStat(EItemStat::EIS_Dex, NewValue); }
//	void SetConstitution(float NewValue) { SetBaseStat(EItemStat::EIS_Con, NewValue); }
//	//추가 스탯 
//	void SetAdditionalStat(EItemStat StatType, float NewValue) { CharacterData.SetAdditionalStat(StatType, NewValue); }
//
//	// 개별 스탯 Getter (편의 함수)
//	FORCEINLINE float GetStrength() const { return CharacterData.GetStrength(); }
//	FORCEINLINE float GetIntelligence() const { return CharacterData.GetIntelligence(); }
//	FORCEINLINE float GetDexterity() const { return CharacterData.GetDexterity(); }
//	FORCEINLINE float GetConstitution() const { return CharacterData.GetConstitution(); }
//
//protected:
//	// 이벤트 델리게이트
//	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
//	FOnHealthChangedDelegate OnHealthChanged;
//	
//	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
//	FOnManaChangedDelegate OnManaChanged;
//	
//	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
//	FOnExpChangedDelegate OnExpChanged;
//	
//	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
//	FOnLevelUpDelegate OnLevelUp;
//
//	// 기본 스탯 변경 델리게이트
//	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
//	FOnBaseStatChangedDelegate OnBaseStatChanged;
//
//	// 전투 스탯 변경 델리게이트
//	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
//	FOnAttackPowerChangedDelegate OnAttackPowerChanged;
//	
//	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
//	FOnDefenseChangedDelegate OnDefenseChanged;
//
//	// 캐릭터 기본 데이터 (체력, 마나)
//	UPROPERTY(EditAnywhere, Category = "Character")
//	FCharacterData CharacterData;
//
//	// 레벨 및 경험치 데이터
//	UPROPERTY(EditAnywhere, Category = "Character")
//	FLevelData LevelData;
//
//	// 전투 스탯 데이터
//	UPROPERTY(EditAnywhere, Category = "Combat")
//	FCombatStats CombatStats;
//
//	// 현재 캐릭터 ID (플레이어용, 레벨업 시 사용)
//	UPROPERTY()
//	FName CurrentCharacterID;
//
//	// 초기화 상태
//	UPROPERTY()
//	bool bIsInitialized = false;
//
//public:
//	// 초기화 완료 델리게이트
//	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
//	FOnStatComponentInitializedDelegate OnStatComponentInitialized;
//
//public:
//	// 델리게이트 접근자
//	FOnHealthChangedDelegate& GetOnHealthChanged() { return OnHealthChanged; }
//	FOnManaChangedDelegate& GetOnManaChanged() { return OnManaChanged; }
//	FOnExpChangedDelegate& GetOnExpChanged() { return OnExpChanged; }
//	FOnLevelUpDelegate& GetOnLevelUp() { return OnLevelUp; }
//	FOnBaseStatChangedDelegate& GetOnBaseStatChanged() { return OnBaseStatChanged; }
//	FOnAttackPowerChangedDelegate& GetOnAttackPowerChanged() { return OnAttackPowerChanged; }
//	FOnDefenseChangedDelegate& GetOnDefenseChanged() { return OnDefenseChanged; }
//	FOnStatComponentInitializedDelegate& GetOnStatComponentInitialized() { return OnStatComponentInitialized; }
}; 