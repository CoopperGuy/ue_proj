// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Animation/DiaMonInstance.h"
#include "Monster/DiaMonster.h"
#include "Kismet/KismetMathLibrary.h"

UDiaMonInstance::UDiaMonInstance()
{
}

void UDiaMonInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	ADiaMonster* diaMonster = Cast<ADiaMonster>(TryGetPawnOwner());
	if (!IsValid(diaMonster))
		return;

	Velocity = diaMonster->GetVelocity();
	Speed = Velocity.Size();
	bIsMoving = Speed > 3.f;

	if (bIsMoving)
	{
		FRotator actorRotation = diaMonster->GetActorRotation();
		FVector forwardVector = UKismetMathLibrary::GetForwardVector(actorRotation);

		FVector normalizedVelocity = Velocity.GetSafeNormal();
		float dotProduct = FVector::DotProduct(forwardVector, normalizedVelocity);
		float crossProduct = FVector::CrossProduct(forwardVector, normalizedVelocity).Z;

		MovementDirection = UKismetMathLibrary::DegAcos(dotProduct);
		if (crossProduct < 0)
			MovementDirection *= -1.f;
	}
}
