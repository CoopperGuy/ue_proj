// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "DiaAbilityTask_JumpToLocation.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnJumpToLocationFinished)
DECLARE_MULTICAST_DELEGATE(FOnJumpToLocationInterrupted)
/**
 * 
 */
UCLASS()
class ARPG_API UDiaAbilityTask_JumpToLocation : public UAbilityTask
{
	GENERATED_BODY()
	
public:
    // 2. 정적 생성 함수 (어빌리티에서 호출할 노드)
    UFUNCTION()
    static UDiaAbilityTask_JumpToLocation* CreateJumpToLocationTask(UGameplayAbility* OwningAbility, const FVector& TargetLocation, float Duration);

    // 3. 실행 진입점
    virtual void Activate() override;

	FOnJumpToLocationFinished OnJumpToLocationFinished; 
	FOnJumpToLocationInterrupted OnJumpToLocationInterrupted;
protected:
    // 4. 정리 작업
    virtual void OnDestroy(bool bInOwnerFinished) override;

private:
    

	FVector StartLocation;
    FVector TargetLocation;
    float TaskDuration;

};
