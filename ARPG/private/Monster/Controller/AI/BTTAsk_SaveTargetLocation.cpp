// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTAsk_SaveTargetLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Monster/Controller/AI/BlackboardKeys.h"

UBTTAsk_SaveTargetLocation::UBTTAsk_SaveTargetLocation(const FObjectInitializer& ObjectInitializer)
{
	NodeName = TEXT("Save Target Location");
	bNotifyTick = false;

	BlackboardKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTAsk_SaveTargetLocation, BlackboardKey));
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTAsk_SaveTargetLocation, TargetActorKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTAsk_SaveTargetLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackboardComp))
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!IsValid(TargetActor))
	{
		BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), FVector::ZeroVector);
		return EBTNodeResult::Failed;
	}

	FVector TargetLocation = TargetActor->GetActorLocation();
	BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), TargetLocation);

	return EBTNodeResult::Succeeded;
}
