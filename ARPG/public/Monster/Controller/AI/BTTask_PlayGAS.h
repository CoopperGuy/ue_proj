// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
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

protected:

	//스킬의 재생한 애니메이션의 SKillID를 가져야한다
	//스킬의 정보를 manager에서 가지고 있으므로
	//해당 정보를 가져와서 재생
    UPROPERTY(EditAnywhere, Category = "PlaySkill")
	int32 SkillID = 0;
};
