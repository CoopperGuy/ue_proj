// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "UBTD_DistanceInRange.generated.h"

/**
 * 
 */
UCLASS(EditInlineNew, meta = (DisplayName ="Distance In Range"))
class ARPG_API UUBTD_DistanceInRange : public UBTDecorator
{
	GENERATED_BODY()
public:
	UUBTD_DistanceInRange();

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector BlackboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float MinDistance = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaxDistance = 200.0f;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
