// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Combatable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPG_API ICombatable
{
	GENERATED_BODY()

public:
	// 공격 가능 상태 체크
	virtual bool CanAttack() const = 0;

	// 현재 전투 상태 체크
	virtual bool IsInCombat() const = 0;	
};
