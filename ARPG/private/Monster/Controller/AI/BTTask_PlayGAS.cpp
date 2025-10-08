// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_PlayGAS.h"
#include "Monster/Controller/AI/BlackboardKeys.h"
#include "Monster/Controller/AI/EAIActionRequest.h"

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
	bNotifyTick = true; // 틱 활성화
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
		return EBTNodeResult::Failed;
	}

	int32 SkillID = (OwnerComp.GetBlackboardComponent()->GetValueAsInt(BlackboardKeys::Monster::ActionRequest));
	if (SkillID == 0)
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
#if WITH_EDITOR
		const bool bCan = UDiaGASHelper::CanActivateAbilityBySkillID(ASC, SkillID);
		if (!bCan)
		{
			UE_LOG(LogTemp, Warning, TEXT("BTTask_PlayGAS: Cannot activate ability for SkillID %d"), SkillID);
			return EBTNodeResult::Failed;
		}
#endif
		if (ASC && UDiaGASHelper::TryActivateAbilityBySkillID(ASC, SkillID))
		{
			CurrentAbilityHandle = Spec->Handle;
			
			AbilityEndedHandle = ASC->AbilityEndedCallbacks.AddUObject(this, &UBTTask_PlayGAS::OnAbilityEnded);
			
			return EBTNodeResult::InProgress; 
		}
	}

	return EBTNodeResult::Failed;
}

void UBTTask_PlayGAS::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	
	// 어빌리티가 여전히 활성화되어 있는지 확인
	ADiaAIController* AIController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(AIController))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ADiaMonster* DiaMonster = Cast<ADiaMonster>(AIController->GetPawn());
	if (!IsValid(DiaMonster))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UAbilitySystemComponent* ASC = DiaMonster->GetAbilitySystemComponent();
	if (!ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 현재 어빌리티가 더 이상 활성화되어 있지 않으면 완료
	FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(CurrentAbilityHandle);
	if (!Spec || !Spec->IsActive())
	{
		// 어빌리티가 완료됨 - SkillID 초기화하고 태스크 완료
		OwnerComp.GetBlackboardComponent()->SetValueAsInt(BlackboardKeys::Monster::ActionRequest, 0);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_PlayGAS::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ADiaAIController* AIController = Cast<ADiaAIController>(OwnerComp.GetAIOwner());
	if (IsValid(AIController))
	{
		ADiaMonster* DiaMonster = Cast<ADiaMonster>(AIController->GetPawn());
		if (IsValid(DiaMonster))
		{
			UAbilitySystemComponent* ASC = DiaMonster->GetAbilitySystemComponent();
			if (ASC && AbilityEndedHandle.IsValid())
			{
				ASC->AbilityEndedCallbacks.Remove(AbilityEndedHandle);
				AbilityEndedHandle.Reset();
			}
		}
	}
	
	OwnerComp.GetBlackboardComponent()->SetValueAsInt(BlackboardKeys::Monster::ActionRequest, 0);
	
	return EBTNodeResult::Aborted;
}

void UBTTask_PlayGAS::OnAbilityEnded(UGameplayAbility* AbilityEndedData)
{
	if (AbilityEndedData->GetCurrentAbilitySpecHandle() == CurrentAbilityHandle)
	{
		if (AbilityEndedHandle.IsValid())
		{
			if (UAbilitySystemComponent* ASC = AbilityEndedData->GetAbilitySystemComponentFromActorInfo())
			{
				ASC->AbilityEndedCallbacks.Remove(AbilityEndedHandle);
			}
			AbilityEndedHandle.Reset();
		}	
	}
}
