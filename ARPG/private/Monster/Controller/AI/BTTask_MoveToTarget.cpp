// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_MoveToTarget.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"
#include "Monster/Controller/AI/BlackboardKeys.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"


#include "DiaBaseCharacter.h"

UBTTask_MoveToTarget::UBTTask_MoveToTarget()
{
	NodeName = TEXT("Move To Target Actor");
	BlackboardKey.SelectedKeyName = BlackboardKeys::Monster::TargetActor;

    BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToTarget, BlackboardKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* aiController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(aiController))
        return EBTNodeResult::Failed;

    ADiaMonster* diaMonster = Cast<ADiaMonster>(aiController->GetPawn());
    if (!IsValid(diaMonster))
        return EBTNodeResult::Failed;

	ADiaBaseCharacter* TargetActor = Cast<ADiaBaseCharacter>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKey.SelectedKeyName));
    if (!IsValid(TargetActor))
        return EBTNodeResult::Failed;

    EPathFollowingRequestResult::Type res = aiController->MoveToActor(TargetActor, 150.0f, true, true);

	return EBTNodeResult::Type();
}
