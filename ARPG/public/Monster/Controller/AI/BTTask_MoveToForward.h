// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToForward.generated.h"


struct FBTMoveToForwardTaskMemory
{
	float ElapsedTime = 0.f;
	FVector LastPos;
};
/**
 * 
 */
UCLASS()
class ARPG_API UBTTask_MoveToForward : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_MoveToForward();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTMoveToForwardTaskMemory); }
protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	float MoveDistance = 300.f;
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxTime = 1.25f;
	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptableRadius = 10.f;
};
