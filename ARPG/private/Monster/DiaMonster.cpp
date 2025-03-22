// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/DiaMonster.h"
#include "Components/CapsuleComponent.h"
#include "Monster/Controller/DiaAIController.h"

#include "GameMode/DungeonGameMode.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "UI/HUDWidget.h"

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
	
	//// CombatStatsComponent를 통해 데미지 처리
	//// 현재 체력 확인 (필요한 경우)
	//float CurrentHealth = 0.0f;
	//float MaxHealth = 0.0f;

	//// 피격 이펙트 재생
	//if (hitEffect)
	//{
	//	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	//		GetWorld(),
	//		hitEffect,
	//		GetActorLocation(),
	//		GetActorRotation()
	//	);
	//}
	//
	//// 피격 사운드 재생
	//if (hitSound)
	//{
	//	UGameplayStatics::PlaySoundAtLocation(this, hitSound, GetActorLocation());
	//}
	//
	//// 히트 리액션 몽타주 재생
	//if (hitReactionMontage && CurrentHealth > 0.0f)
	//{
	//	PlayCharacterMontage(hitReactionMontage);
	//}
	//
	//// 사망 처리
	//if (CurrentHealth <= 0.0f)
	//{
	//	PlayDieAnimation();
	//}
	
	return ActualDamage;
}

void ADiaMonster::UpdateHPGauge(float CurHealth, float MaxHelath)
{
    ADungeonGameMode* DungeonGameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
    
    // 로그 추가 (디버깅용)
    if (!DungeonGameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHPGauge: DungeonGameMode is null"));
        return;
    }
    
    UHUDWidget* HUD = DungeonGameMode->GetHUDWidget();
    if (HUD)
    {
        float HPPersentage = CurHealth / MaxHelath;
        HUD->UpdateMonsterPercentage(BarType::BT_HP, HPPersentage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHPGauge: HUD Widget is null"));
    }
}

void ADiaMonster::PlayDieAnimation()
{
	Super::PlayDieAnimation();
}

void ADiaMonster::Die()
{
	Super::Die();
}