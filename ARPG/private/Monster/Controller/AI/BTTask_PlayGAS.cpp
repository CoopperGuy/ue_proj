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
#include "DiaComponent/DiaSkillManagerComponent.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"


UBTTask_PlayGAS::UBTTask_PlayGAS()
{
	NodeName = TEXT("Player GameplayAbilitySystemSkill");
	bNotifyTick = true; // 틱 활성화

	BlackboardKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PlayGAS, BlackboardKey));

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

	int32 SkillID = (OwnerComp.GetBlackboardComponent()->GetValueAsInt(BlackboardKey.SelectedKeyName));
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
	if (DiaMonster && DiaMonster->GetSkillManagerComponent() && DiaMonster->GetSkillManagerComponent()->TryActivateAbilityBySkillID(SkillID))
	{
		CurrentAbilityHandle = Spec->Handle;
		bAbilityEnded = false; // 플래그 초기화
		
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

	// 어빌리티 종료 플래그 확인
	if (bAbilityEnded)
	{
		// 어빌리티가 완료됨 - SkillID 초기화하고 태스크 완료
		int32 CompletedSkillID = OwnerComp.GetBlackboardComponent()->GetValueAsInt(BlackboardKey.SelectedKeyName);
		OwnerComp.GetBlackboardComponent()->SetValueAsInt(BlackboardKey.SelectedKeyName, 0);
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
	
	int32 AbortedSkillID = OwnerComp.GetBlackboardComponent()->GetValueAsInt(BlackboardKey.SelectedKeyName);
	UE_LOG(LogTemp, Log, TEXT("BTTask_PlayGAS: Task aborted. Resetting ActionRequest from SkillID %d to 0"), AbortedSkillID);
	OwnerComp.GetBlackboardComponent()->SetValueAsInt(BlackboardKey.SelectedKeyName, 0);
	
	return EBTNodeResult::Aborted;
}

void UBTTask_PlayGAS::OnAbilityEnded(UGameplayAbility* AbilityEndedData)
{
	if (AbilityEndedData && AbilityEndedData->GetCurrentAbilitySpecHandle() == CurrentAbilityHandle)
	{
		// 콜백 제거
		if (AbilityEndedHandle.IsValid())
		{
			if (UAbilitySystemComponent* ASC = AbilityEndedData->GetAbilitySystemComponentFromActorInfo())
			{
				ASC->AbilityEndedCallbacks.Remove(AbilityEndedHandle);
			}
			AbilityEndedHandle.Reset();
		}
		
		// 태스크 완료는 TickTask에서 처리하도록 플래그 설정
		bAbilityEnded = true;
	}
}
