// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AnimInstance/DiaAniminstance.h"
#include "Character/DiaCharacter.h"
#include "Kismet/KismetMathLibrary.h"

UDiaAniminstance::UDiaAniminstance()
{
}

void UDiaAniminstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ADiaCharacter* diaCharacter = Cast<ADiaCharacter>(TryGetPawnOwner());
	if (!IsValid(diaCharacter))
		return;

	Velocity = diaCharacter->GetVelocity();
	Speed = Velocity.Size();
	bIsMoving = Speed > 3.f;

	if(bIsMoving)
	{
		FRotator actorRotation = diaCharacter->GetActorRotation();
		FVector forwardVector = UKismetMathLibrary::GetForwardVector(actorRotation);

		FVector normalizedVelocity = Velocity.GetSafeNormal();
		float dotProduct = FVector::DotProduct(forwardVector, normalizedVelocity);
		float crossProduct = FVector::CrossProduct(forwardVector, normalizedVelocity).Z;

		MovementDirection = UKismetMathLibrary::DegAcos(dotProduct);
		if (crossProduct < 0)
			MovementDirection *= -1.f;
	}
}
