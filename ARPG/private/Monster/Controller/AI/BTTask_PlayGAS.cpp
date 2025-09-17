// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_PlayGAS.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "AbilitySystemComponent.h"
#include "GAS/DiaGASHelper.h"
#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "System/GASSkillManager.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"


UBTTask_PlayGAS::UBTTask_PlayGAS()
{
	NodeName = TEXT("Player GameplayAbilitySystemSkill");
}

EBTNodeResult::Type UBTTask_PlayGAS::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ADiaAIController* AIController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController)) return EBTNodeResult::Aborted;

	ADiaMonster* DiaMonster = Cast<ADiaMonster>(AIController->GetPawn());
	if (!IsValid(DiaMonster)) return EBTNodeResult::Aborted;

	UAbilitySystemComponent* ASC = DiaMonster->GetAbilitySystemComponent();
	if (!ASC || !ASC->AbilityActorInfo.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask] ASC not ready (null or no ActorInfo)."));
		return EBTNodeResult::Failed;
	}

	// 스킬이 실제로 부여됐는지 먼저 확인
	if (FGameplayAbilitySpec* Spec = UDiaGASHelper::GetAbilitySpecBySkillID(ASC, SkillID))
	{
		if (Spec->IsActive())
		{
			return EBTNodeResult::InProgress;
		}
#if WITH_EDITOR
		FGameplayTagContainer Failure;
		const bool bCan = Spec->Ability
			? Spec->Ability->CanActivateAbility(Spec->Handle, ASC->AbilityActorInfo.Get(), nullptr, nullptr, &Failure)
			: false;
		UE_LOG(LogTemp, Log, TEXT("[BTTask] SkillID=%d CanActivate=%d Failure=%s Active=%d"),
			SkillID, bCan, *Failure.ToString(), Spec->IsActive());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask] Spec not found for SkillID=%d (미부여)."), SkillID);
		// 필요시 클래스 기반 폴백:
		// return ASC->TryActivateAbilityByClass(ChosenAbilityClass) ? Succeeded : Failed;
#endif
	}

	if (ASC && UDiaGASHelper::TryActivateAbilityBySkillID(ASC, SkillID))
	{
		UE_LOG(LogTemp, Log, TEXT("GAS 스킬 실행 성공 - ID: %d"), SkillID);
		return EBTNodeResult::Succeeded;
	}
	UE_LOG(LogTemp, Warning, TEXT("GAS 스킬 실행 실패 - ID: %d"), SkillID);
	return EBTNodeResult::Failed;
}
