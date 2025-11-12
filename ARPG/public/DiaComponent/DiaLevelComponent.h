// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DiaLevelComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUp, uint32, NewLevel);

UCLASS()
class ARPG_API UDiaLevelComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaLevelComponent();

protected:
	virtual void BeginPlay() override;

public:	

	void LevelUp();

	FOnLevelUp OnLevelUpDelegate;
private:
	uint32 Level = 1;

public:
	uint32 GetLevel() const { return Level; }
};
