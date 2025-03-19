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
    // Blackboard 키 지정 (이동할 위치를 저장할 키)
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

    // 네비게이션 시스템 가져오기
    UNavigationSystemV1* navSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
    if (!navSystem)
        return EBTNodeResult::Failed;

    // 홈 위치 가져오기 (스폰 위치)
    FVector homeLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector("HomeLocation");

    // 랜덤한 위치 찾기
    FNavLocation randomLocation;
    bool bFound = false;

    // 적절한 위치를 찾을 때까지 시도
    for (int32 Tries = 0; Tries < 10; Tries++)
    {
        if (navSystem->GetRandomReachablePointInRadius(homeLocation, patrolRadius, randomLocation))
        {
            // 홈으로부터의 거리 체크
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

    // 찾은 위치를 블랙보드에 설정
    OwnerComp.GetBlackboardComponent()->SetValueAsVector(
        GetSelectedBlackboardKey(),
        randomLocation.Location
    );

    // 디버그 시각화
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
