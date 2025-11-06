// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Skill/DiaSkillManager.h"
#include "Types/DiaMonsterTable.h"
#include "DiaCombatComponent.generated.h"


class ADiaSkillBase;
class ADiaProjectileSkill;
class UDiaStatusEffect;
class UDiaDamageType;
class ADiaBaseCharacter;

// 전투 상태 열거형
UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Idle UMETA(DisplayName = "Idle"),           // 전투 중이 아님
	InCombat UMETA(DisplayName = "InCombat"),       // 전투 중
	Retreating UMETA(DisplayName = "Retreating"),     // 전투에서 빠져나오는 중
	Dead UMETA(DisplayName = "Dead")            // 사망
};

//// 델리게이트 선언
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillExecutedDelegate, int32, SkillID);
//// 전투 이벤트 델리게이트
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChangedDelegate, ECombatState, NewState, ECombatState, OldState);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageTakenDelegate, float, DamageAmount, AActor*, DamageCauser);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageDealtDelegate, float, DamageAmount, AActor*, DamageTarget);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathDelegate);




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UDiaCombatComponent : public UActorComponent
{
	GENERATED_BODY()

//public:	
//	UDiaCombatComponent();
//
//	// 전투 프로세스 관련
//	void EnterCombat(AActor* CombatTarget = nullptr);
//	void ExitCombat();
//	// 사망 처리
//	void HandleDeath();
//
//	//데미지 관련 
//	float ApplyDamage(AActor* Target, AActor* DamageCauser, float BaseDamage, TSubclassOf<UDiaDamageType> DamageTypeClass = nullptr);
//	void ReceiveDamage(float DamageAmount, AActor* DamageCauser);
//	
//	//어그로 관리
//	void AddThreatToActor(AActor* Actor, float ThreatAmount);
//	void ClearAllThreats();
//protected:
//	// 엔진 기본 함수
//	virtual void BeginPlay() override;
//	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//
//	void InitializeSkills();
//
//	// 전투 타이머 업데이트
//	void UpdateCombatTimer(float DeltaTime);
//	// 전투 타임아웃 처리
//	void HandleCombatTimeout();
//
//	float CalculateExpReward() const;
//	void GiveExperienceToKiller();
//	// 스탯 컴포넌트 접근 함수
//public:
//	//////////////////////////////////////////////////////////////////////////
//	// 전투 관련 함수 
//	//////////////////////////////////////////////////////////////////////////
//	// 
//	// 기본 공격 실행
//	UFUNCTION(Category = "Combat")
//	bool ExecuteBasicAttack();
//
//	// 공격 가능 여부 확인
//	bool CanExecuteAttack() const;
//
//	UFUNCTION()
//	float CalculateDamage(float BaseDamage);
//
//	// 스킬 실행
//	UFUNCTION()
//	bool ExecuteSkill(int32 SkillID);
//	// 스킬 등록
//	UFUNCTION()
//	bool RegisterSkill(int32 SkillID);
//
//	// 스킬 해제
//	UFUNCTION()
//	void UnregisterSkill(int32 SkillID);
//
//	//스킬 소모값 사용
//	UFUNCTION()
//	bool ProcessSkillCost(ADiaSkillBase* Skill);
//
//	/// <summary>
//	/// 델리게이트 함수들
//	/// ->아직 미사용
//	/// </summary>
//	/// <param name="StatusEffect"></param>
//	UFUNCTION()
//	void OnStatusEffectAdded(UDiaStatusEffect* StatusEffect);
//
//	UFUNCTION()
//	void OnStatusEffectRemoved(UDiaStatusEffect* StatusEffect);
//
//	//ui관련 
//	UFUNCTION()
//	void OnUpdateHpGauge(float CurHP, float MaxHP);
//
//	//사망 처리
//	UFUNCTION()
//	void OnDeathProcess();
//   //// 스텟 변경 통지
//   // UFUNCTION()
//   // void OnStatChanged(EStatType StatType, float NewValue);
//protected:
//	//로직 관련
//	AActor* FindNearestPlayer();
//private:
//	bool bCanAttack = false;
//
///// <summary>
///// Getter Setter
///// </summary>
//public:
//
//	// 스킬 관련 함수
//	TArray<int32> GetActiveSkillIDs() const;
//	ADiaSkillBase* GetSkillByID(int32 SkillID) const;
//
//	// 전투 상태 관리
//	void SetCombatState(ECombatState NewState);
//	
//	ECombatState GetCombatState() const { return CurrentCombatState; }
//	
//	bool IsInCombat() const { return CurrentCombatState == ECombatState::InCombat; }
//	
//	bool IsDead() const { return CurrentCombatState == ECombatState::Dead; }
//	
//	// 타겟 관리
//	void SetCurrentTarget(AActor* NewTarget);
//	
//	AActor* GetCurrentTarget() const { return CurrentTarget; }
//	
//	AActor* GetHighestThreatActor() const;
//
//protected:
//	// 이벤트 델리게이트
//	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
//	FOnCombatStateChangedDelegate OnCombatStateChanged;
//	
//	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
//	FOnDamageTakenDelegate OnDamageTaken;
//	
//	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
//	FOnDamageDealtDelegate OnDamageDealt;
//	
//	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
//	FOnDeathDelegate OnDeath;
//	
//	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
//	FOnSkillExecutedDelegate OnSkillExecuted;
//
//	// 위협 테이블 (액터 -> 위협도)
//	UPROPERTY()
//	TMap<AActor*, float> ThreatTable;
//
//	UPROPERTY(EditAnywhere, Category = "Combat|Animation")
//	UAnimMontage* BasicAttackMontage;
//
//	// 스킬 맵 (스킬 ID -> 스킬 인스턴스)
//	UPROPERTY()
//	TMap<int32, ADiaSkillBase*> ActiveSkills;
//
//	// 스킬 클래스 맵 (스킬 ID -> 스킬 클래스)
//	UPROPERTY(EditDefaultsOnly, Category = "Combat|Skills")
//	TMap<int32, TSubclassOf<ADiaSkillBase>> SkillClasses;
//
//	// 기본 공격 실행 중 여부
//	bool bIsAttacking;
//
//	// 공격 쿨다운 타이머
//	FTimerHandle AttackCooldownTimer;
//
//	// 스킬 매니저 레퍼런스
//	UPROPERTY()
//	UDiaSkillManager* SkillManager;
//
//	// 기본 공격 스킬 ID
//	UPROPERTY(EditDefaultsOnly, Category = "Combat|Skills")
//	int32 BasicAttackSkillID = 1;
//
//	// 전투 상태
//	UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
//	ECombatState CurrentCombatState;
//	
//	// 전투 타이머
//	UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
//	float CombatTimer;
//	
//	// 전투 타임아웃 (이 시간 동안 전투 행동이 없으면 전투 종료)
//	UPROPERTY(EditDefaultsOnly, Category = "Combat|Settings")
//	float CombatTimeout = 5.0f;
//	
//	// 현재 타겟
//	UPROPERTY(BlueprintReadOnly, Category = "Combat|Targets")
//	AActor* CurrentTarget;
//	
//	// 죽인 캐릭터
//	UPROPERTY(BlueprintReadOnly, Category = "Combat|Targets")
//	ADiaBaseCharacter* Killer;

};
