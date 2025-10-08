// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTAsk_SaveTargetLocation.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UBTTAsk_SaveTargetLocation : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTTAsk_SaveTargetLocation(const FObjectInitializer& ObjectInitializer);
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	// 타겟 액터를 가져올 블랙보드 키 (에디터에서 선택 가능)
	UPROPERTY(EditAnywhere, Category = Blackboard)
	FBlackboardKeySelector TargetActorKey;
	
};
