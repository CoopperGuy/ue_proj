// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_RotateAndActiveSkill.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "AbilitySystemComponent.h"
#include "GAS/DiaGASHelper.h"
#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "System/GASSkillManager.h"
#include "DiaComponent/DiaSkillManagerComponent.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"



UBTTask_RotateAndActiveSkill::UBTTask_RotateAndActiveSkill(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Rotate And Active Skill");
}

EBTNodeResult::Type UBTTask_RotateAndActiveSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type res = Super::ExecuteTask(OwnerComp, NodeMemory);

	if(res == EBTNodeResult::Succeeded)
	{
		ADiaAIController* AIController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
		if (!IsValid(AIController)) return EBTNodeResult::Aborted;

		ADiaMonster* DiaMonster = Cast<ADiaMonster>(AIController->GetPawn());
		if (!IsValid(DiaMonster)) return EBTNodeResult::Aborted;

		UAbilitySystemComponent* ASC = DiaMonster->GetAbilitySystemComponent();
		if (!ASC || !ASC->AbilityActorInfo.IsValid())
		{
			return EBTNodeResult::Failed;
		}

		// 스킬이 실제로 부여됐는지 먼저 확인
		if (FGameplayAbilitySpec* Spec = UDiaGASHelper::GetAbilitySpecBySkillID(ASC, SkillID))
		{
			if (Spec->IsActive())
			{
				return EBTNodeResult::InProgress;
			}
			if (DiaMonster && DiaMonster->GetSkillManagerComponent() && DiaMonster->GetSkillManagerComponent()->TryActivateAbilityBySkillID(SkillID))
			{
				return EBTNodeResult::Succeeded;
			}
		}
	}

	return res;
}

void UBTTask_RotateAndActiveSkill::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{

	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

}
