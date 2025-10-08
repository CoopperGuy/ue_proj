// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/Decorater/BTDecorator_CheckActionRequest.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTDecorator_CheckActionRequest::UBTDecorator_CheckActionRequest()
{
	NodeName = TEXT("Check Action Request");
	BlackboardKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckActionRequest, BlackboardKey));
}

bool UBTDecorator_CheckActionRequest::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	int32 ActionRequest = OwnerComp.GetBlackboardComponent()->GetValueAsInt(GetSelectedBlackboardKey());
	if (ActionRequest != 0)
	{
		return true;
	}
	return false;
}
