// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_FindRandomPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UBTTask_FindRandomPatrolPoint : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTask_FindRandomPatrolPoint();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    // 순찰 범위 설정
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float patrolRadius = 500.0f;

    // 홈 위치로부터의 최소 거리
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float minDistanceFromHome = 100.0f;

    // 디버그 그리기 여부
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowDebugLines = true;
};
