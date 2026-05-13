// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/AI/BTTask_ChangeMovmentSpeed.h"
#include "Monster/Controller/AI/BlackboardKeys.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"
#include "AbilitySystemComponent.h"
#include "GAS/DiaAttributeSet.h"

UBTTask_ChangeMovmentSpeed::UBTTask_ChangeMovmentSpeed()
{
	NodeName = TEXT("Change Movement Max Speed");
	BlackboardKey.SelectedKeyName = BlackboardKeys::Movement::MaxSpeed;
}

EBTNodeResult::Type UBTTask_ChangeMovmentSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = (OwnerComp.GetAIOwner());
	if (!IsValid(aiController))
		return EBTNodeResult::Failed;

	ADiaMonster* DiaMonster = Cast<ADiaMonster>(aiController->GetPawn());
	if (!IsValid(DiaMonster)) return EBTNodeResult::Failed;

	if (UAbilitySystemComponent* ASC = DiaMonster->GetAbilitySystemComponent())
	{
		ASC->SetNumericAttributeBase(UDiaAttributeSet::GetMovementSpeedAttribute(), DefaultSpeed);
	}
	else if (DiaMonster->GetCharacterMovement())
	{
		DiaMonster->GetCharacterMovement()->MaxWalkSpeed = DefaultSpeed;
	}

	return EBTNodeResult::Succeeded;
}
