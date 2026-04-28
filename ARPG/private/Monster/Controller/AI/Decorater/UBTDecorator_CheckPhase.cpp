// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/Decorater/UBTDecorator_CheckPhase.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"

UUBTDecorator_CheckPhase::UUBTDecorator_CheckPhase()
{
	NodeName = TEXT("Check Phase");
	BlackboardKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UUBTDecorator_CheckPhase, BlackboardKey));
	CheckPhase = 1; // 기본적으로 쿨다운 체크 활성화
}

bool UUBTDecorator_CheckPhase::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// 쿨다운 체크를 하는 경우, 스킬이 실제로 사용 가능한지 확인
	ADiaAIController* AIController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController))
	{
		return false;
	}

	ADiaMonster* DiaMonster = Cast<ADiaMonster>(AIController->GetPawn());
	if (!IsValid(DiaMonster))
	{
		return false;
	}

	if (DiaMonster->GetPhase() == CheckPhase)
	{
		return true;
	}

	return false;
}
