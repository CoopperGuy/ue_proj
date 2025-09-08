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
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float patrolRadius = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float minDistanceFromHome = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowDebugLines = true;
};
