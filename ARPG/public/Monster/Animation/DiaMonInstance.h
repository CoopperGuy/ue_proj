// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DiaMonInstance.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaMonInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UDiaMonInstance();
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MovementDirection;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dead")
	bool bIsDead = false;

	bool bIsMoving = false;

	
};
