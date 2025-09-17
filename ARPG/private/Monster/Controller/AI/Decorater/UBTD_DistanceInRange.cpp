// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/Decorater/UBTD_DistanceInRange.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

#include "Monster/Controller/DiaAIController.h"

UUBTD_DistanceInRange::UUBTD_DistanceInRange()
{
    NodeName = TEXT("Distance In Range");
    
    // Blackboard Key 설정
    BlackboardKey.SelectedKeyName = TEXT("TargetActor");
}

bool UUBTD_DistanceInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
    if (!Pawn)
    {
        return false;
    }

    const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AActor* Target = Cast<AActor>(BB->GetValueAsObject(BlackboardKey.SelectedKeyName));
    if (!Target)
    {
        return false;
    }

    const float Dist = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());
    return (Dist >= MinDistance && Dist <= MaxDistance);
}
