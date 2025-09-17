// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_MoveToLastPos.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"

#include "Monster/Controller/AI/BlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"

UBTTask_MoveToLastPos::UBTTask_MoveToLastPos()
{
	NodeName = TEXT("Move To Last Position");
	// Accept only vector types
	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToLastPos, BlackboardKey));
}

EBTNodeResult::Type UBTTask_MoveToLastPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FVector LastPos = OwnerComp.GetBlackboardComponent()->GetValueAsVector(BlackboardKey.SelectedKeyName);

	if(LastPos.IsNearlyZero())
		return EBTNodeResult::Failed;

	AAIController* aiController = (OwnerComp.GetAIOwner());
	if (!IsValid(aiController))
		return EBTNodeResult::Failed;

	aiController->MoveToLocation(LastPos, 10.f);

	return EBTNodeResult::Succeeded;
}
