// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaBaseCharacter.h"
#include "DiaComponent/DiaCombatComponent.h"
#include "DiaComponent/DiaStatusEffectComponent.h"
#include "DiaComponent/DiaStatComponent.h"
#include "GAS/DiaAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ADiaBaseCharacter::ADiaBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 전투 컴포넌트 생성 및 초기화
	CombatComponent = CreateDefaultSubobject<UDiaCombatComponent>(TEXT("CombatComponent"));
	
	// 스탯 컴포넌트 생성 및 초기화
	StatsComponent = CreateDefaultSubobject<UDiaStatComponent>(TEXT("StatsComponent"));

	// 상태 이상 효과 컴포넌트 생성
	StatusEffectComponent = CreateDefaultSubobject<UDiaStatusEffectComponent>(TEXT("StatusEffectComponent"));

	// GAS 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// AttributeSet 생성
	AttributeSet = CreateDefaultSubobject<UDiaAttributeSet>(TEXT("AttributeSet"));

	Tags.Add(FName(TEXT("Character")));
}

// Called when the game starts or when spawned
void ADiaBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize Ability System
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	
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
	GrantInitialGASAbilities();
	// 초기 스킬 등록
}

void ADiaBaseCharacter::GrantInitialGASAbilities()
{

}

float ADiaBaseCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	// 전투 컴포넌트에 데미지 전달
	// - > 이제 gas로 변경한다

	//if (IsValid(CombatComponent))
	//{
	//	CombatComponent->ReceiveDamage(ActualDamage, DamageCauser);
	//}
	
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

	//SetLifeSpan(3.0f);
}

void ADiaBaseCharacter::AddExp(float ExpAmount)
{
	StatsComponent->AddExperience(ExpAmount);
}

void ADiaBaseCharacter::SetGravity(bool bEnableGravityAndCollision)
{
   UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: CharacterMovementComponent not found!"), *FString(__FUNCTION__));
		return;
	}

	// 캡슐 컴포넌트 가져오기
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule)
	{
		 UE_LOG(LogTemp, Warning, TEXT("%s: CapsuleComponent not found!"), *FString(__FUNCTION__));
		 return;
	}

	if (bEnableGravityAndCollision)
	{
		// --- 중력 및 기본 충돌 활성화 ---
		UE_LOG(LogTemp, Log, TEXT("%s: Enabling Gravity and Default Collision/Movement"), *FString(__FUNCTION__));

		// 1. 이동 모드 복원: 기본적으로 'Falling' 상태로 설정하여 중력 및 지면 감지 활성화
		MoveComp->SetMovementMode(MOVE_Walking);
		// 만약 특정 기본 모드(예: Walking)를 원하거나 이전 상태를 저장/복원해야 한다면 추가 로직 필요

		// 2. 중력 스케일 복원: 기본값인 1.0 또는 원래 값으로 설정
		MoveComp->GravityScale = 1.0f; // 필요시 클래스 멤버 변수에 기본값 저장 후 사용

		// 3. 캡슐 콜리전 활성화: 쿼리와 물리 충돌 모두 가능하도록 설정
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// 4. 콜리전 프로파일 복원: 가장 확실한 방법은 원래 프로파일 이름으로 복원하는 것
		// Capsule->SetCollisionProfileName(DefaultCollisionProfileName); // BeginPlay 등에서 DefaultCollisionProfileName 저장 필요
		// 또는: 수동으로 모든 채널에 대한 반응을 기본값(Block)으로 설정
		Capsule->SetCollisionResponseToAllChannels(ECR_Block);
	}
	else
	{
		// --- 중력 및 충돌 영향 비활성화 ("유령 모드" 와 유사) ---
		UE_LOG(LogTemp, Log, TEXT("%s: Disabling Gravity and Collision Effects"), *FString(__FUNCTION__));

		// 1. 이동 모드 변경: 'Flying' 모드로 설정하여 중력 및 지면 충돌 무시
		MoveComp->SetMovementMode(MOVE_Flying);

		// 2. 중력 스케일 0 설정: 중력의 영향을 완전히 제거
		MoveComp->GravityScale = 0.0f;

		// 3. 현재 속도 정지: 부유 상태로 만들기 위해 현재 이동 중지 (선택적)
		MoveComp->Velocity = FVector::ZeroVector;
		MoveComp->StopMovementImmediately(); // 즉시 이동 멈춤

		// 4. 캡슐 콜리전 비활성화: 모든 물리적 충돌 및 쿼리를 무시 (벽 통과 가능)
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// 만약 오버랩 이벤트나 라인 트레이스는 감지해야 한다면 QueryOnly 사용
		// Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		 Capsule->SetCollisionResponseToAllChannels(ECR_Overlap); // QueryOnly 사용 시 모든 채널 오버랩으로 설정 가능
	}
}

void ADiaBaseCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CurrentMontage == Montage)
	{
		CurrentMontage = nullptr;
	}
}

UAbilitySystemComponent* ADiaBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

