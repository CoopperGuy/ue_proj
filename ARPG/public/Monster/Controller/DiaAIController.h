// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense.h"

#include "Skill/DiaDamageType.h"

#include "DiaAIController.generated.h"


class UBehaviorTree;
class UBlackboardData;
class ADiaMonster;
class UAISenseConfig_Sight;
class UAIPerceptionComponent;
/**
 * 
 */
UCLASS()
class ARPG_API ADiaAIController : public AAIController
{
	GENERATED_BODY()
public:
	ADiaAIController();

    // 전투 상태 업데이트
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void UpdateCombatState();
    
    // 타겟 업데이트
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void UpdateTarget();
    
    // 공격 실행
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void ExecuteAttack();
    
    // 타겟 설정
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    void SetTarget(AActor* NewTarget);
    
    // 현재 타겟 가져오기
    UFUNCTION(BlueprintCallable, Category = "AI|Combat")
    const AActor* GetCurrentTarget() const { return CurrentTarget; }
    
    // 타겟 감지
    void DetectTargets();
    
    // 가장 가까운 플레이어 찾기
    AActor* FindNearestPlayer();
    
    // 타겟까지의 거리 계산
    float GetDistanceToTarget() const;
    
    // 소유한 몬스터 가져오기
    ADiaMonster* GetControlledMonster() const;

    void InitBehaviorTree(UBehaviorTree* _behaiviortree);
    void InitBlackBoardData(APawn* InPawn, UBlackboardData* blackboardData);
    void InitializeAI();
protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);


protected:
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree* behaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBlackboardData* blackboardData;

    static const FName TargetKey;
    static const FName PatrolLocationKey;
    static const FName IsInAttackRangeKey;

    // 현재 타겟
    UPROPERTY(BlueprintReadOnly, Category = "AI|Combat")
    AActor* CurrentTarget;
    
    // 타겟 감지 거리
    UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
    float DetectionRadius = 1000.0f;
    
    // 공격 가능 거리
    UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
    float AttackRange = 150.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	UAIPerceptionComponent* AIPerceptionComp;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Combat")
	UAISenseConfig_Sight* SightConfig;

	bool bIsLineOfSight = false;

    UPROPERTY()
	FVector LastSeenLocation;
};
