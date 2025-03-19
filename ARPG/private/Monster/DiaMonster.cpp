// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/DiaMonster.h"
#include "Components/CapsuleComponent.h"
#include "Monster/Controller/DiaAIController.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"

ADiaMonster::ADiaMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	//Default Monster AIController
	AIControllerClass = ADiaAIController::StaticClass();

	Tags.Add(FName(TEXT("Monster")));
}

void ADiaMonster::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ADiaMonster::CanAttack() const
{
	return false;
}

bool ADiaMonster::IsInCombat() const
{
	return false;
}

void ADiaMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADiaMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


float ADiaMonster::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	// CombatStatsComponent를 통해 데미지 처리
	// 현재 체력 확인 (필요한 경우)
	float CurrentHealth = 0.0f;
	float MaxHealth = 0.0f;

	// 피격 이펙트 재생
	if (hitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			hitEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
	
	// 피격 사운드 재생
	if (hitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, hitSound, GetActorLocation());
	}
	
	// 히트 리액션 몽타주 재생
	if (hitReactionMontage && CurrentHealth > 0.0f)
	{
		PlayCharacterMontage(hitReactionMontage);
	}
	
	// 사망 처리
	if (CurrentHealth <= 0.0f)
	{
		PlayDieAnimation();
	}
	
	return ActualDamage;
}

void ADiaMonster::PlayDieAnimation()
{
	Die();
}

void ADiaMonster::Die()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(3.0f);
}