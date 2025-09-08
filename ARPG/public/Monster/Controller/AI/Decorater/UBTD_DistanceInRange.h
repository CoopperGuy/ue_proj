// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "UBTD_DistanceInRange.generated.h"

/**
 * 
 */
UCLASS(EditInlineNew, meta = (DisplayName ="Distance In Range"))
class ARPG_API UUBTD_DistanceInRange : public UBTDecorator_Blackboard
{
	GENERATED_BODY()
public:
	UUBTD_DistanceInRange();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MinDistance = 0.f;
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxDistance = 200.f;
};
