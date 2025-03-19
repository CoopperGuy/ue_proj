// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Skill/DiaSkillManager.h"
#include "Skill/DiaSkillType.h"
#include "DiaCombatComponent.generated.h"


class ADiaSkillBase;
class ADiaProjectileSkill;
class UDiaStatusEffect;

USTRUCT(BlueprintType)
struct ARPG_API FCharacterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Mana = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxMana = 100.f;
};

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
};

// 전투 상태 열거형
UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,           // 전투 중이 아님
    InCombat,       // 전투 중
    Retreating,     // 전투에서 빠져나오는 중
    Dead            // 사망
};

// 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManaChangedDelegate, float, NewMana, float, MaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillExecutedDelegate, int32, SkillID);
// 전투 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChangedDelegate, ECombatState, NewState, ECombatState, OldState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTakenDelegate, float, DamageAmount, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageDealtDelegate, float, DamageAmount, AActor*, DamageTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UDiaCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaCombatComponent();

   // 스탯 정보 제공 및 관리 함수
    void ApplyDamage(float DamageAmount);
    bool HasEnoughMana(float ManaAmount) const;
    void ConsumeMana(float ManaAmount);
    void RestoreMana(float ManaAmount);
    void RestoreHealth(float HealthAmount);

    // 전투 타이머 관리
    void EnterCombat(AActor* CombatTarget = nullptr);
    void ExitCombat();

    float ApplyDamage(AActor* Target, float BaseDamage, TSubclassOf<UDiaDamageType> DamageTypeClass = nullptr);
    void ReceiveDamage(float DamageAmount, AActor* DamageCauser);
    // 사망 처리
    void HandleDeath();

    void AddThreatToActor(AActor* Actor, float ThreatAmount);
    void ClearAllThreats();

    // 델리게이트 인스턴스
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnHealthChangedDelegate OnHealthChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnManaChangedDelegate OnManaChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnSkillExecutedDelegate OnSkillExecuted;
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    // 전투 타이머 업데이트
    void UpdateCombatTimer(float DeltaTime);
    
    // 전투 타임아웃 처리
    void HandleCombatTimeout();

public:	
    // 기본 공격 실행
    UFUNCTION(Category = "Combat")
    bool ExecuteBasicAttack();

    // 공격 가능 여부 확인
    bool CanExecuteAttack() const;

    UFUNCTION()
    float CalculateDamage(float BaseDamage);

    // 스킬 실행
    UFUNCTION()
    bool ExecuteSkill(int32 SkillID);
    // 스킬 등록
    UFUNCTION()
    bool RegisterSkill(int32 SkillID);

    // 스킬 해제
    UFUNCTION()
    void UnregisterSkill(int32 SkillID);

    //스킬 소모값 사용
    UFUNCTION()
    bool ProcessSkillCost(ADiaSkillBase* Skill);

    UFUNCTION()
    void OnStatusEffectAdded(UDiaStatusEffect* StatusEffect);

    UFUNCTION()
    void OnStatusEffectRemoved(UDiaStatusEffect* StatusEffect);

   //// 스텟 변경 통지
   // UFUNCTION()
   // void OnStatChanged(EStatType StatType, float NewValue);
protected:
    UPROPERTY(EditAnywhere, Category = "Combat")
    FCombatStats CombatStats;

    UPROPERTY(EditAnywhere, Category = "Combat|Animation")
    UAnimMontage* BasicAttackMontage;

    // 스킬 맵 (스킬 ID -> 스킬 인스턴스)
    UPROPERTY()
    TMap<int32, ADiaSkillBase*> ActiveSkills;

    // 스킬 클래스 맵 (스킬 ID -> 스킬 클래스)
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Skills")
    TMap<int32, TSubclassOf<ADiaSkillBase>> SkillClasses;

    // 기본 공격 실행 중 여부
    bool bIsAttacking;

    // 공격 쿨다운 타이머
    FTimerHandle AttackCooldownTimer;

    // 캐릭터 데이터
    UPROPERTY(EditAnywhere, Category = "Character")
    FCharacterData CharacterData;

    // 스킬 매니저 레퍼런스
    UPROPERTY()
    UDiaSkillManager* SkillManager;

    // 기본 공격 스킬 ID
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Skills")
    int32 BasicAttackSkillID = 1;

    // 스킬 초기화
    void InitializeSkills();

    // 위협 테이블 (액터 -> 위협도)
    UPROPERTY()
    TMap<AActor*, float> ThreatTable;

    AActor* FindNearestPlayer();
private:
    bool bCanAttack = false;
public:
    // 스탯 정보 가져오기

    const FCharacterData& GetCharacterData() const { return CharacterData; }
    void SetCharacterData(const FCharacterData& NewData) { CharacterData = NewData; }
    FORCEINLINE float GetCurrentHealth() const { return CharacterData.Health; }
    FORCEINLINE float GetMaxHealth() const { return CharacterData.MaxHealth; }
    FORCEINLINE float GetCurrentMana() const { return CharacterData.Mana; }
    FORCEINLINE float GetMaxMana() const { return CharacterData.MaxMana; }
    FORCEINLINE float GetAttackPower() const { return CombatStats.AttackPower; }
    FORCEINLINE float GetAttackSpeed() const { return CombatStats.AttackSpeed; }
    FORCEINLINE float GetAttackRange() const { return CombatStats.AttackRange; }
    
    // 스탯 정보 설정
    void SetMaxHealth(float NewMaxHealth);
    void SetMaxMana(float NewMaxMana);

    // 스킬 관련 함수
    TArray<int32> GetActiveSkillIDs() const;
    ADiaSkillBase* GetSkillByID(int32 SkillID) const;

    // 전투 상태 관리
    void SetCombatState(ECombatState NewState);
    
    ECombatState GetCombatState() const { return CurrentCombatState; }
    
    bool IsInCombat() const { return CurrentCombatState == ECombatState::InCombat; }
    
    bool IsDead() const { return CurrentCombatState == ECombatState::Dead; }
    

    
    
    // 타겟 관리
    void SetCurrentTarget(AActor* NewTarget);
    
    AActor* GetCurrentTarget() const { return CurrentTarget; }
    
    
    AActor* GetHighestThreatActor() const;
    
    // 이벤트 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnCombatStateChangedDelegate OnCombatStateChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDamageTakenDelegate OnDamageTaken;
    
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDamageDealtDelegate OnDamageDealt;
    
    UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
    FOnDeathDelegate OnDeath;
    
protected:
    // 전투 상태
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    ECombatState CurrentCombatState;
    
    // 전투 타이머
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    float CombatTimer;
    
    // 전투 타임아웃 (이 시간 동안 전투 행동이 없으면 전투 종료)
    UPROPERTY(EditDefaultsOnly, Category = "Combat|Settings")
    float CombatTimeout = 5.0f;
    
    // 현재 타겟
    UPROPERTY(BlueprintReadOnly, Category = "Combat|Targets")
    AActor* CurrentTarget;
    
    
};
