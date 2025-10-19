// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/Decorater/BTDecorator_CheckActionRequest.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "AbilitySystemComponent.h"
#include "GAS/DiaGASHelper.h"
#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"

UBTDecorator_CheckActionRequest::UBTDecorator_CheckActionRequest()
{
	NodeName = TEXT("Check Action Request");
	BlackboardKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckActionRequest, BlackboardKey));
	bCheckCooldown = true; // 기본적으로 쿨다운 체크 활성화
}

bool UBTDecorator_CheckActionRequest::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	int32 ActionRequest = OwnerComp.GetBlackboardComponent()->GetValueAsInt(GetSelectedBlackboardKey());
	
	// ActionRequest가 없으면 바로 실패
	if (ActionRequest == 0)
	{
		return false;
	}

	// 쿨다운 체크를 하지 않는 경우, ActionRequest만 확인
	if (!bCheckCooldown)
	{
		return true;
	}

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

	UAbilitySystemComponent* ASC = DiaMonster->GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	// DiaGASHelper를 사용하여 스킬 사용 가능 여부 확인
	// (IsActive, Cooldown, Cost 모두 체크)
	bool bCanActivate = UDiaGASHelper::CanActivateAbilityBySkillID(ASC, ActionRequest);
	
	if (!bCanActivate)
	{
		UE_LOG(LogTemp, Warning, TEXT("[CheckActionRequest] SkillID %d is requested but cannot be activated (cooldown or other reason)"), ActionRequest);
	}
	
	return bCanActivate;
}
