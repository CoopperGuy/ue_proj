// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_MoveToForward.h"
#include "Monster/Controller/DiaAIController.h"
UBTTask_MoveToForward::UBTTask_MoveToForward()
{
	NodeName = TEXT("Move To Forward");

	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToForward::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTMoveToForwardTaskMemory* TaskMemory = reinterpret_cast<FBTMoveToForwardTaskMemory*>(NodeMemory);
	if (!TaskMemory)
		return EBTNodeResult::Failed;

	AAIController* aiController = (OwnerComp.GetAIOwner());
	if (!IsValid(aiController))
		return EBTNodeResult::Failed;

	APawn* Pawn = aiController->GetPawn();
	if (!IsValid(Pawn))
		return EBTNodeResult::Failed;

	FVector ForwardVector = Pawn->GetActorForwardVector();
	TaskMemory->LastPos = Pawn->GetActorLocation() + ForwardVector * MoveDistance;

	aiController->MoveToLocation(TaskMemory->LastPos, AcceptableRadius * 0.5f);

	return EBTNodeResult::InProgress;
}

void UBTTask_MoveToForward::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTMoveToForwardTaskMemory* TaskMemory = reinterpret_cast<FBTMoveToForwardTaskMemory*>(NodeMemory);
	if(!TaskMemory)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	TaskMemory->ElapsedTime += DeltaSeconds;
	if(TaskMemory->ElapsedTime >= MaxTime)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
		
	AAIController* aiController = (OwnerComp.GetAIOwner());
	if (!IsValid(aiController))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	float DistanceToTarget = FVector::Dist(aiController->GetPawn()->GetActorLocation(), TaskMemory->LastPos);
	if (DistanceToTarget <= AcceptableRadius)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
