// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_CheckActionRequest.generated.h"

/**
 * ActionRequest 값이 있는지 확인하고,
 * 옵션으로 해당 스킬이 실제로 사용 가능한지(쿨다운, 코스트 등)도 체크할 수 있습니다.
 */
UCLASS()
class ARPG_API UBTDecorator_CheckActionRequest : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTDecorator_CheckActionRequest();
	
protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	/**
	 * true면 ActionRequest 값뿐만 아니라 해당 스킬의 쿨다운, 코스트 등도 체크합니다.
	 * false면 ActionRequest != 0만 체크합니다.
	 */
	UPROPERTY(EditAnywhere, Category = "Skill", meta = (DisplayName = "Check Cooldown"))
	bool bCheckCooldown = true;
};
