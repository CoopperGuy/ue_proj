// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "NotifyMelee.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UNotifyMelee : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UNotifyMelee();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
	
};
