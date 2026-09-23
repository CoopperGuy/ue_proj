// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_MoveToTarget.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"
#include "Monster/Controller/AI/BlackboardKeys.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GAS/DiaGameplayTags.h"
#include "Navigation/PathFollowingComponent.h"

#include "DiaBaseCharacter.h"

UBTTask_MoveToTarget::UBTTask_MoveToTarget()
{
	NodeName = TEXT("Move To Target Actor");
	BlackboardKey.SelectedKeyName = BlackboardKeys::Monster::TargetActor;

    BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveToTarget, BlackboardKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ADiaAIController* aiController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
    if (!IsValid(aiController))
        return EBTNodeResult::Failed;

    ADiaMonster* diaMonster = Cast<ADiaMonster>(aiController->GetPawn());
    if (!IsValid(diaMonster))
        return EBTNodeResult::Failed;

    // The ability owns the entire wind-up, hit and recovery, including montage blend-out.
    // Root-motion charge steps run inside that ability and do not use this chase task.
    if (diaMonster->IsAttackInProgress())
        return EBTNodeResult::Failed;

    const UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
        return EBTNodeResult::Failed;

	ADiaBaseCharacter* TargetActor = Cast<ADiaBaseCharacter>(Blackboard->GetValueAsObject(BlackboardKey.SelectedKeyName));
    if (!IsValid(TargetActor))
        return EBTNodeResult::Failed;

    if (UAbilitySystemComponent* ASC = diaMonster->GetAbilitySystemComponent())
    {
        // 이동 불가 상태 체크
        if (ASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Charging)
            || ASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_KnockBack))
        {
            return EBTNodeResult::Failed; // 또는 Succeeded
        }
    }


    const float ApproachDistance = diaMonster->GetCombatApproachDistance();
    if (FVector::DistSquared(diaMonster->GetActorLocation(), TargetActor->GetActorLocation())
        <= FMath::Square(ApproachDistance))
    {
        aiController->StopMovement();
        return EBTNodeResult::Succeeded;
    }

    // MoveToActor adds both capsule radii to its reach test. Keep the table's
    // center-to-center distance consistent with DistanceInRange decorators.
    const float CapsuleRadii = diaMonster->GetCapsuleComponent()->GetScaledCapsuleRadius()
        + TargetActor->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float AcceptanceRadius = FMath::Max(0.f, ApproachDistance - CapsuleRadii);
    const EPathFollowingRequestResult::Type Result = aiController->MoveToActor(
        TargetActor, AcceptanceRadius, true, true);

    return Result == EPathFollowingRequestResult::Failed
        ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;
}
