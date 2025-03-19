// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DiaAniminstance.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaAniminstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UDiaAniminstance();
	
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MovementDirection;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	FVector Velocity;

	bool bIsMoving = false;
};
