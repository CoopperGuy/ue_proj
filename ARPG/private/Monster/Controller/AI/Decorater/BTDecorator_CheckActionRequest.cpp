// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/Decorater/BTDecorator_CheckActionRequest.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "AbilitySystemComponent.h"

#include "GAS/DiaGameplayTags.h"
#include "DiaComponent/DiaSkillManagerComponent.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"
#include "Logging/ARPGLogChannels.h"

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

	bool bIsStunned = ASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Stunned);
	if (bIsStunned)
	{
		return false;
	}

	// Apply the same skill runtime preparation used by the actual activation path.
	UDiaSkillManagerComponent* SkillManager = DiaMonster->GetSkillManagerComponent();
	if (!IsValid(SkillManager))
	{
		return false;
	}

	bool bCanActivate = SkillManager->CanActivateAbilityBySkillID(ActionRequest);
	
	if (!bCanActivate)
	{
		UE_LOG(LogARPG, Warning, TEXT("[CheckActionRequest] SkillID %d is requested but cannot be activated (cooldown or other reason)"), ActionRequest);
	}
	
	return bCanActivate;
}
