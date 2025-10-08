// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Monster/Controller/AI/EAIActionRequest.h"
#include "BTTask_RotateToTarget.generated.h"


struct FBTRotateToTargetMemory
{
	FVector TargetDirection;
	bool bIsValid;
};

/**
 * 
 */
UCLASS()
class ARPG_API UBTTask_RotateToTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_RotateToTarget(const FObjectInitializer& ObjectInitializer);

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	//NodeMemory 크기 설정
	//안하면 크래시남
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTRotateToTargetMemory); }
protected:
	// 회전 속도 (도/초)
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RotationSpeed = 360.0f;

	// 정밀도 (도 단위, 이 각도 이내면 완료로 판단)
	UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float Precision = 5.0f;

	// 목표 방향 계산
	FVector GetTargetDirection(UBehaviorTreeComponent& OwnerComp, const FVector& TargetLocation) const;

	UPROPERTY(EditAnywhere, Category = "Blackboard|GAS")
	FBlackboardKeySelector ActionRequest;

	UPROPERTY(EditAnywhere, Category = "Blackboard|GAS")
	int32 SkillID = 0;
};
