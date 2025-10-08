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
	ADiaAIController* AIController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
	if (IsValid(AIController))
	{
		ADiaMonster* DiaMonster = Cast<ADiaMonster>(AIController->GetPawn());
		if (IsValid(DiaMonster))
		{
			UAbilitySystemComponent* ASC = DiaMonster->GetAbilitySystemComponent();
			if (ASC && SkillID != 0)
			{
				if (FGameplayAbilitySpec* Spec = UDiaGASHelper::GetAbilitySpecBySkillID(ASC, SkillID))
				{
					auto Ability = Spec->Ability;
					if (Ability)
					{
						bool bCoolDown = Ability->CheckCooldown(Spec->Handle, ASC->AbilityActorInfo.Get());
						bool bCost = Ability->CheckCost(Spec->Handle, ASC->AbilityActorInfo.Get());
						if(Spec->IsActive() == false && bCoolDown && bCost)
						{
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}
