// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "UBTDecorator_CheckPhase.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UUBTDecorator_CheckPhase : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
public:
	UUBTDecorator_CheckPhase();
	
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	UPROPERTY(EditAnywhere, Category = "Phase", meta = (DisplayName = "Check Phase"))
	int32 CheckPhase = 0;
};
