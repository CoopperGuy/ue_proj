// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/Decorater/BTDecorator_IsValidSkill.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "AbilitySystemComponent.h"
#include "GAS/DiaGASHelper.h"
#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "System/GASSkillManager.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"

UBTDecorator_IsValidSkill::UBTDecorator_IsValidSkill()
{
	NodeName = TEXT("IsValidSkill");
	SkillID = 0;
}

bool UBTDecorator_IsValidSkill::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	//현재 사용중인 스킬이 있는지 확인
	int32 SelectedSkillID = (OwnerComp.GetBlackboardComponent()->GetValueAsInt(BlackboardKey.SelectedKeyName));
	//스킬 아이디 확인
	//이미 스킬을 사용중이면 false
	if (SelectedSkillID != 0)
	{
		return false;
	}
	if (SkillID == 0)
	{
		return false;
	}

	ADiaAIController* AIController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
	if (IsValid(AIController))
	{
		ADiaMonster* DiaMonster = Cast<ADiaMonster>(AIController->GetPawn());
		if (IsValid(DiaMonster))
		{
			UAbilitySystemComponent* ASC = DiaMonster->GetAbilitySystemComponent();
			if (ASC && SkillID != 0)
			{
				// DiaGASHelper의 CanActivateAbilityBySkillID를 사용하여
				// IsActive, Cooldown, Cost를 모두 체크
				return UDiaGASHelper::CanActivateAbilityBySkillID(ASC, SkillID);
			}
		}
	}
	return false;
}
