// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaBaseCharacter.h"
#include "DiaComponent/DiaCombatComponent.h"
#include "DiaComponent/DiaStatusEffectComponent.h"
#include "Components/CapsuleComponent.h"

ADiaBaseCharacter::ADiaBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 전투 스탯 컴포넌트 생성 및 초기화
	CombatStatsComponent = CreateDefaultSubobject<UDiaCombatComponent>(TEXT("CombatStatsComponent"));
	
	// 상태 이상 효과 컴포넌트 생성
	StatusEffectComponent = CreateDefaultSubobject<UDiaStatusEffectComponent>(TEXT("StatusEffectComponent"));

	Tags.Add(FName(TEXT("Character")));
}

// Called when the game starts or when spawned
void ADiaBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SetupInitialSkills();
}

// Called every frame
void ADiaBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADiaBaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ADiaBaseCharacter::SetupInitialSkills()
{
	if (!IsValid(CombatStatsComponent) || !GetWorld()) return;

	// 초기 스킬 등록
	for (const auto& _SkillID : InitialSkills)
	{
		// 스킬 ID를 통해 스킬 등록
		CombatStatsComponent->RegisterSkill(_SkillID);
	}
}

float ADiaBaseCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	// 전투 컴포넌트에 데미지 전달
	if (IsValid(CombatStatsComponent))
	{
		CombatStatsComponent->ReceiveDamage(ActualDamage, DamageCauser);
	}
	
	return ActualDamage;
}



float ADiaBaseCharacter::PlayCharacterMontage(UAnimMontage* MontageToPlay, float PlayRate)
{
	if (!IsValid(GetMesh())) return 0.f;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (!IsValid(MontageToPlay) || !IsValid(AnimInstance))
	{
		return 0.0f;
	}

	// 현재 재생중인 몽타주가 있다면 중단
	if (IsValid(CurrentMontage) && CurrentMontage != MontageToPlay)
	{
		StopCharacterMontage(0.1f);
	}

	// 새로운 몽타주 재생
	//float Duration = AnimInstance->Montage_Play(MontageToPlay, PlayRate);
	AnimInstance->PlaySlotAnimationAsDynamicMontage(MontageToPlay, FName("DefaultSlot"), 0.25f, 0.25f);

	CurrentMontage = MontageToPlay;

	// 몽타주 종료 시 호출될 델리게이트 바인딩
	//if (Duration > 0.f)
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ADiaBaseCharacter::OnMontageEnded);
	}

	return 0.f;
}

void ADiaBaseCharacter::StopCharacterMontage(float BlendOutTime)
{
	if (!IsValid(GetMesh()) || !IsValid(GetMesh()->GetAnimInstance()))
	{
		return;
	}

	GetMesh()->GetAnimInstance()->Montage_Stop(BlendOutTime, CurrentMontage);
	CurrentMontage = nullptr;
}

bool ADiaBaseCharacter::IsPlayingMontage(UAnimMontage* Montage) const
{
	if (IsValid(GetMesh()) && IsValid(GetMesh()->GetAnimInstance()))
	{
		return GetMesh()->GetAnimInstance()->Montage_IsPlaying(Montage);
	}
	return false;
}

void ADiaBaseCharacter::StopAnimMontage(UAnimMontage* Montage)
{
	if (IsValid(GetMesh()) && IsValid(GetMesh()->GetAnimInstance()))
	{
		GetMesh()->GetAnimInstance()->Montage_Stop(0.25f, Montage);
	}
}

void ADiaBaseCharacter::UpdateHPGauge(float CurHealth, float MaxHelath)
{
}

void ADiaBaseCharacter::PlayDieAnimation()
{
	Die();
}

void ADiaBaseCharacter::Die()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(3.0f);
}

void ADiaBaseCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CurrentMontage == Montage)
	{
		CurrentMontage = nullptr;
	}
}

