// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/DiaSkillActor.h"
#include "DiaGroundObj.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API ADiaGroundObj : public ADiaSkillActor
{
	GENERATED_BODY()
	
public:
	ADiaGroundObj();
	virtual void BeginPlay() override;
	virtual void Initialize(const FGASSkillData& SkillData, AActor* InOwner, UAbilitySystemComponent* InSourceASC = nullptr, TSubclassOf<UGameplayEffect> InDamageEffect = nullptr) override;

protected:
	void OnHitDetect();
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skill|Ground")
	float Radius = 200.f;
};
