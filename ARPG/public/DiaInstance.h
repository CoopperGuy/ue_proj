// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DiaInstance.generated.h"

class UDiaSkillManager;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaInstance : public UGameInstance
{
	GENERATED_BODY()
public:
    virtual void Init() override;

    // 스킬 매니저 접근자
    UFUNCTION(BlueprintCallable, Category = "Managers")
    UDiaSkillManager* GetSkillManager() const { return SkillManager; }

protected:
    // 각종 매니저들
    UPROPERTY()
    UDiaSkillManager* SkillManager;

	
	
};
