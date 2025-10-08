// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster/Controller/AI/BTTask_RotateToTarget.h"
#include "BTTask_RotateAndActiveSkill.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UBTTask_RotateAndActiveSkill : public UBTTask_RotateToTarget
{
	GENERATED_BODY()
	
public:
	UBTTask_RotateAndActiveSkill(const FObjectInitializer& ObjectInitializer);

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
