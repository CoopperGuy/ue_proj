// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AbilityTask/DiaAbilityTask_JumpToLocation.h"
#include "DiaBaseCharacter.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UDiaAbilityTask_JumpToLocation* UDiaAbilityTask_JumpToLocation::CreateJumpToLocationTask(UGameplayAbility* OwningAbility, const FVector& TargetLocation, float Duration)
{
	UDiaAbilityTask_JumpToLocation* MyObj = NewAbilityTask<UDiaAbilityTask_JumpToLocation>(OwningAbility);
	MyObj->TaskDuration = Duration;

	return MyObj;
}

void UDiaAbilityTask_JumpToLocation::Activate()
{
	ADiaBaseCharacter* BaseCharacter = Cast< ADiaBaseCharacter>(Ability->GetAvatarActorFromActorInfo());
	if (!IsValid(BaseCharacter))
		return;

	UCharacterMovementComponent* MoveComp = BaseCharacter->GetCharacterMovement();
	if(!IsValid(MoveComp))
		return;

	MoveComp->Launch(TargetLocation - BaseCharacter->GetActorLocation());
}

void UDiaAbilityTask_JumpToLocation::OnDestroy(bool bInOwnerFinished)
{
}
