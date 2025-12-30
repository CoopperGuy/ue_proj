// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/DiaSkillObject.h"
#include "DiaGroundObj.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API ADiaGroundObj : public ADiaSkillObject
{
	GENERATED_BODY()
	
public:
	ADiaGroundObj();
	virtual void BeginPlay() override;

protected:
	void OnHitDetect();
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skill|Ground")
	float Radius = 200.f;
};
