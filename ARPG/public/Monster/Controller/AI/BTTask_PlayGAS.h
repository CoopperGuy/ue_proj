// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "AbilitySystemComponent.h"
#include "BTTask_PlayGAS.generated.h"


struct FBTPlayGASTaskMemory
{
	// 현재 실행 중인 어빌리티의 핸들을 저장
	FGameplayAbilitySpecHandle CurrentAbilityHandle;
	int32 SkillID = 0;

};
/**
 * 
 */
UCLASS()
class ARPG_API UBTTask_PlayGAS : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTTask_PlayGAS();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTPlayGASTaskMemory); }
};
