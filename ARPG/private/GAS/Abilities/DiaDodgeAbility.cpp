// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/DiaDodgeAbility.h"
#include "DiaBaseCharacter.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "GAS/DiaGameplayTags.h"
UDiaDodgeAbility::UDiaDodgeAbility()
{
	ActivationOwnedTags.AddTag(FDiaGameplayTags::Get().State_Invincible);
}

void UDiaDodgeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//입력키 이동 방향 가져옴
	ADiaBaseCharacter* DiaCharacter = Cast<ADiaBaseCharacter>(ActorInfo->AvatarActor.Get());
	FVector MoveDir = DiaCharacter->GetLastMovementInputVector();
	
	//입력 없을 때 처리
	if(MoveDir.IsNearlyZero())
	{
		//캐릭터 바라보는 방향으로 회피
		MoveDir = DiaCharacter->GetActorForwardVector();
	}

	MoveDir.Normalize();

	UAbilityTask_ApplyRootMotionConstantForce* RootMotionTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this,
		TEXT("Dodge"),
		MoveDir,
		DodgeStrength, // Strength
		DodgeDuration,   // Duration
		true,   // bIsAdditive
		nullptr,
		ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity,
		FVector::ZeroVector,
		0.0f,
		false
	);

	RootMotionTask->OnFinish.AddDynamic(this, &UDiaDodgeAbility::OnRollFinish);
	RootMotionTask->ReadyForActivation();
}

void UDiaDodgeAbility::OnRollFinish()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

