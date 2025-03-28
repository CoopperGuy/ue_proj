// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_FindRandomPatrolPoint.h"
#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

#include "DrawDebugHelpers.h"

UBTTask_FindRandomPatrolPoint::UBTTask_FindRandomPatrolPoint()
{
    NodeName = TEXT("Find Patrol Random Location");
    BlackboardKey.SelectedKeyName = TEXT("PatrolLocation");
}

EBTNodeResult::Type UBTTask_FindRandomPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* aiController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
    if (!aiController)
        return EBTNodeResult::Failed;

    ADiaMonster* diaMonster = Cast<ADiaMonster>(aiController->GetPawn());
    if (!diaMonster)
        return EBTNodeResult::Failed;

    UNavigationSystemV1* navSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
    if (!navSystem)
        return EBTNodeResult::Failed;

    FVector homeLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector("HomeLocation");

    FNavLocation randomLocation;
    bool bFound = false;

    for (int32 Tries = 0; Tries < 10; Tries++)
    {
        if (navSystem->GetRandomReachablePointInRadius(homeLocation, patrolRadius, randomLocation))
        {
            float distanceFromHome = FVector::Dist(homeLocation, randomLocation.Location);
            if (distanceFromHome >= minDistanceFromHome)
            {
                bFound = true;
                break;
            }
        }
    }

    if (!bFound)
        return EBTNodeResult::Failed;

    OwnerComp.GetBlackboardComponent()->SetValueAsVector(
        GetSelectedBlackboardKey(),
        randomLocation.Location
    );

    if (bShowDebugLines)
    {
        DrawDebugSphere(
            GetWorld(),
            randomLocation.Location,
            20.0f,
            16,
            FColor::Green,
            false,
            2.0f
        );

        DrawDebugLine(
            GetWorld(),
            diaMonster->GetActorLocation(),
            randomLocation.Location,
            FColor::Green,
            false,
            2.0f
        );
    }

    return EBTNodeResult::Succeeded;
}
