// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_Sparkle.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UBTService_Sparkle : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_Sparkle();
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
