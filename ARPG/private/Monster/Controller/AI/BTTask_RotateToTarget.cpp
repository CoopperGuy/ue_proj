// Fill out your copyright notice in the Description page of Project Settings.

#include "Monster/Controller/AI/BTTask_RotateToTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "Monster/Controller/AI/BlackboardKeys.h"

#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"


UBTTask_RotateToTarget::UBTTask_RotateToTarget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = "Rotate to Target (Smooth)";
	bNotifyTick = true;  
	
	RotationSpeed = 360.0f;
	Precision = 5.0f;  // 적당한 정밀도로 설정

	ActionRequest.SelectedKeyName = BlackboardKeys::Monster::ActionRequest;
	ActionRequest.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_RotateToTarget, ActionRequest));
	SkillID = 0;
}

EBTNodeResult::Type UBTTask_RotateToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* aiController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(aiController))
		return EBTNodeResult::Failed;
		
	APawn* pawn = aiController->GetPawn();
	if (!IsValid(pawn))
		return EBTNodeResult::Failed;

	// Blackboard에서 TargetActor 가져오기
	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::Monster::TargetActor));
	if (!IsValid(TargetActor))
	{
		return EBTNodeResult::Failed;
	}
	
	FVector TargetLocation = TargetActor->GetActorLocation();

	// 타겟 방향 계산
	FVector TargetDirection = GetTargetDirection(OwnerComp, TargetLocation);
	if (TargetDirection.IsNearlyZero())
	{
		return EBTNodeResult::Failed;
	}

	// 메모리 설정
	FBTRotateToTargetMemory* MyMemory = reinterpret_cast<FBTRotateToTargetMemory*>(NodeMemory);
	MyMemory->TargetDirection = TargetDirection;
	MyMemory->bIsValid = true;

	// 각도 차이 확인
	FRotator CurrentRotation = pawn->GetActorRotation();
	FRotator TargetRotation = TargetDirection.Rotation();
	float AngleDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));

	// 이미 타겟을 바라보고 있다면 즉시 완료
	if (AngleDifference <= Precision)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsInt(ActionRequest.SelectedKeyName, SkillID);
		MyMemory->bIsValid = false;
		return EBTNodeResult::Succeeded;
	}

	// 회전 시작
	return EBTNodeResult::InProgress;
}

void UBTTask_RotateToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTRotateToTargetMemory* MyMemory = reinterpret_cast<FBTRotateToTargetMemory*>(NodeMemory);
	if (!MyMemory->bIsValid)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIController* aiController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(aiController))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}	
	
	APawn* pawn = aiController->GetPawn();
	if (!IsValid(pawn))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Blackboard에서 TargetActor로 방향 업데이트
	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BlackboardKeys::Monster::TargetActor));
	if (IsValid(TargetActor))
	{
		FVector TargetLocation = TargetActor->GetActorLocation();
		FVector NewTargetDirection = GetTargetDirection(OwnerComp, TargetLocation);
		if (!NewTargetDirection.IsNearlyZero())
		{
			MyMemory->TargetDirection = NewTargetDirection;
		}
	}

	// 회전 처리
	FRotator CurrentRotation = pawn->GetActorRotation();
	FRotator TargetRotation = MyMemory->TargetDirection.Rotation();
	float AngleDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));

	// 회전 완료 체크
	if (AngleDifference <= Precision)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsInt(ActionRequest.SelectedKeyName, SkillID);
		MyMemory->bIsValid = false;
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 부드러운 회전
	FRotator NewRotation = CurrentRotation;
	NewRotation.Yaw = FMath::FInterpConstantTo(CurrentRotation.Yaw, TargetRotation.Yaw, DeltaSeconds, RotationSpeed);
	NewRotation.Yaw = FMath::UnwindDegrees(NewRotation.Yaw);
	
	pawn->SetActorRotation(NewRotation);
}

EBTNodeResult::Type UBTTask_RotateToTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTRotateToTargetMemory* MyMemory = reinterpret_cast<FBTRotateToTargetMemory*>(NodeMemory);
	MyMemory->TargetDirection = FVector::ZeroVector;
	MyMemory->bIsValid = false;
	return EBTNodeResult::Aborted;
}


FVector UBTTask_RotateToTarget::GetTargetDirection(UBehaviorTreeComponent& OwnerComp, const FVector& TargetLocation) const
{
	AAIController* aiController = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!IsValid(aiController))
		return FVector::ZeroVector;

	APawn* pawn = aiController->GetPawn();
	if (!IsValid(pawn))
		return FVector::ZeroVector;

	// 폰에서 목표 위치로의 방향 벡터 계산
	FVector ToTarget = TargetLocation - pawn->GetActorLocation();
	ToTarget.Z = 0; // 수평면에서의 방향만 고려
	FVector Direction = ToTarget.GetSafeNormal();

	return Direction;
}
