// Fill out your copyright notice in the Description page of Project Settings.


#include "Notify/AnimNotify_Die.h"
#include "DiaBaseCharacter.h"
#include "Animation/AnimInstance.h"

void UAnimNotify_Die::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!IsValid(MeshComp)) return;

	ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(MeshComp->GetOwner());
	if (Character)
	{
		Character->PauseDeathMontage();
	}
}
