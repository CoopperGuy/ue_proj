// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "AbilitySystemComponent.h"
#include "BTTask_PlayGAS.generated.h"

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

private:
	// 애니메이션 완료를 감지하기 위한 델리게이트 핸들
	FDelegateHandle AbilityEndedHandle;
	
	// 현재 실행 중인 어빌리티의 핸들을 저장
	UPROPERTY()
	FGameplayAbilitySpecHandle CurrentAbilityHandle;
	
	// 어빌리티 완료 콜백
	UFUNCTION()
	void OnAbilityEnded(UGameplayAbility* AbilityEndedData);
};
