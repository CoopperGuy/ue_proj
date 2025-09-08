// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_PlayGAS.h"
#include "BTTask_PlayGAS.h"

UBTTask_PlayGAS::UBTTask_PlayGAS()
{
	NodeName = TEXT("Find Patrol Random Location");
}

EBTNodeResult::Type UBTTask_PlayGAS::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::Type();
}
