// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaBaseCharacter.h"
#include "DiaComponent/DiaLevelComponent.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "GameFramework/PlayerState.h"

#include "GAS/DiaGASHelper.h"
#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "AbilitySystemComponent.h"
#include "System/GASSkillManager.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/DiaGameplayTags.h"

ADiaBaseCharacter::ADiaBaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// GAS 컴포넌트 생성
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// AttributeSet 생성
	AttributeSet = CreateDefaultSubobject<UDiaAttributeSet>(TEXT("AttributeSet"));

	//LevelComponent 생성
	LevelComponent = CreateDefaultSubobject<UDiaLevelComponent>(TEXT("LevelComponent"));

	Tags.Add(FName(TEXT("Character")));
}

// Called when the game starts or when spawned
void ADiaBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void ADiaBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Initialize Ability System
	if (AbilitySystemComponent)
	{
		if(Tags.Contains(FName("Player")))
		{
			APlayerState* PS = GetPlayerState<APlayerState>();
			if (PS)
			{
				AbilitySystemComponent->InitAbilityActorInfo(PS, this);
			}
		}
		else
		{
			AbilitySystemComponent->InitAbilityActorInfo(NewController, this);
		}
		AbilitySystemComponent->AddSpawnedAttribute(AttributeSet);
	}
}

void ADiaBaseCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // 클라이언트에서도 ASC 초기화 보장
    if (AbilitySystemComponent)
    {
        APlayerState* PS = GetPlayerState<APlayerState>();
        if (PS)
        {
            AbilitySystemComponent->InitAbilityActorInfo(PS, this);
            AbilitySystemComponent->AddSpawnedAttribute(AttributeSet);
        }
    }
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


	AbilitySystemComponent->RegisterGameplayTagEvent(
		FDiaGameplayTags::Get().State_Stunned,
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &ADiaBaseCharacter::OnStunTagChanged);

	AbilitySystemComponent->RegisterGameplayTagEvent(
		FDiaGameplayTags::Get().State_Slowed,
		EGameplayTagEventType::AnyCountChange
	).AddUObject(this, &ADiaBaseCharacter::OnSlowTagChanged);
}

void ADiaBaseCharacter::GrantInitialGASAbilities()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrantInitialGASAbilities: No ASC"));
		return;
	}

	UGASSkillManager* GasSkillMgr = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
	if (!GasSkillMgr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrantInitialGASAbilities: No GASSkillManager"));
	}

	int32 MappingIndex = 0;
	if (InitialSkills.Num() > 0)
	{
		for (int32 SkillID : InitialSkills)
		{
			if (MappingIndex >= MaxSkillMapping)
			{
				break;
			}

			//부여에 성공했다면
			bool isGrants = SetUpSkillID(SkillID);		
			if(isGrants)
			{
				if (SkillIDMapping.IsValidIndex(MappingIndex))
				{
					SkillIDMapping[MappingIndex] = SkillID;
				}
				MappingIndex++;
			}
		}
	}


	// 임시 주석 - 불필요한 로그
	//for (const FGameplayAbilitySpec& S : ASC->GetActivatableAbilities())
	//{
	//	UE_LOG(LogTemp, Log, TEXT("[GAS] Granted: Ability=%s, InputID(SkillID)=%d, IsActive=%s"),
	//		*GetNameSafe(S.Ability), S.InputID, S.IsActive() ? TEXT("true") : TEXT("false"));
	//}
}

bool ADiaBaseCharacter::SetUpSkillID(int32 SkillID)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	UGASSkillManager* GasSkillMgr = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
	if (!GasSkillMgr)
	{
		return false;
	}

	TSubclassOf<UGameplayAbility> AbilityClass = nullptr;
	const FGASSkillData* FoundData = nullptr;
	if (GasSkillMgr)
	{
		FoundData = GasSkillMgr->GetSkillDataPtr(SkillID);
		if (FoundData)
		{
			AbilityClass = FoundData->AbilityClass ? FoundData->AbilityClass : nullptr;
		}
	}

	if(!AbilityClass)
	{
		return false;
	}

	bool bGranted = false;
	if (FoundData)
	{
		// AbilityTags가 비어있으면 SkillData의 첫 번째 태그 사용
		FGameplayTag TagToUse;
		if (AbilityTags.Num() > 0)
		{
			TagToUse = AbilityTags.GetGameplayTagArray().Last();
		}
		else if (FoundData->AbilityTags.Num() > 0)
		{
			TagToUse = FoundData->AbilityTags.GetGameplayTagArray()[0];
		}

		// 태그 체크 없이 무조건 부여하도록 수정
		if (TagToUse.IsValid())
		{
			bGranted = UDiaGASHelper::GrantAbilityFromSkillData(ASC, *FoundData, SkillID, TagToUse);
			UE_LOG(LogTemp, Log, TEXT("Granted Ability %s with Tag %s for SkillID %d"), *AbilityClass->GetName(), *TagToUse.ToString(), SkillID);
		}
		else
		{
			// 태그 없어도 부여할 수 있도록 빈 태그로 시도
			bGranted = UDiaGASHelper::GrantAbilityFromSkillData(ASC, *FoundData, SkillID, FGameplayTag());
			UE_LOG(LogTemp, Log, TEXT("Granted Ability %s with No Tag for SkillID %d"), *AbilityClass->GetName(), SkillID);
		}
	}

	return bGranted;
}

void ADiaBaseCharacter::OnStunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		// 스턴 상태 시작
		UE_LOG(LogTemp, Log, TEXT("Character %s is Stunned."), *GetName());
		PlayCharacterMontage(StunMontage);
	}
	else
	{
		// 스턴 상태 종료
		UE_LOG(LogTemp, Log, TEXT("Character %s is No Longer Stunned."), *GetName());
		StopCharacterMontage(0.2f);
	}
}

void ADiaBaseCharacter::OnSlowTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	// 느려짐 상태 변화 처리
	UE_LOG(LogTemp, Log, TEXT("Character %s Slow Tag Changed. New Count: %d"), *GetName(), NewCount);
	if (NewCount > 0)
	{
		FGameplayTagContainer SearchTags;
		SearchTags.AddTag(FDiaGameplayTags::Get().State_Slowed);  // Effect에 붙인 태그
		TArray<FActiveGameplayEffectHandle> Handles = AbilitySystemComponent->GetActiveEffectsWithAllTags(SearchTags);
		float SlowMagnitude = 0.f;
		for(const FActiveGameplayEffectHandle& Handle : Handles)
		{
			const FActiveGameplayEffect* ActiveGE = AbilitySystemComponent->GetActiveGameplayEffect(Handle);
			if (ActiveGE)
			{
				// 느려짐 효과의 크기 가져오기
				float Magnitude = ActiveGE->Spec.GetSetByCallerMagnitude(FDiaGameplayTags::Get().State_Slowed);
				SlowMagnitude += Magnitude;
			}
		}
		GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed * SlowMagnitude; 
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultMovementSpeed; 
	}
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

	if (AnimInstance->OnMontageEnded.IsBound())
	{
		AnimInstance->OnMontageEnded.RemoveDynamic(this, &ADiaBaseCharacter::OnMontageEnded);
	}

	// 새로운 몽타주 재생
	float Duration = AnimInstance->Montage_Play(MontageToPlay, PlayRate);

	// 몽타주 종료 시 호출될 델리게이트 바인딩
	if (Duration > 0.f)
	{
		CurrentMontage = MontageToPlay;
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

///사망시 관리
/// 1. 애니메이션 재생
/// 2. 콜리전 비활성화
/// 3. 일정 시간 후 액터 제거
/// 4. 경험치 분배
void ADiaBaseCharacter::PlayDieAnimation()
{
	if (IsValid(CurrentMontage))
	{
		StopAnimMontage(CurrentMontage);
	}

	if (IsValid(DieMontage))
	{
		PlayCharacterMontage(DieMontage, 1.0f);
	}
}

void ADiaBaseCharacter::Die(ADiaBaseCharacter* Causer)
{
	PlayDieAnimation();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetLifeSpan(3.0f);

	bIsDead = true;

	KillerCharacterWeakPtr = Causer;
}

void ADiaBaseCharacter::SetGravity(bool bEnableGravityAndCollision)
{
   UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: CharacterMovementComponent not found!"), *FString(__FUNCTION__));
		return;
	}

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	if (!Capsule)
	{
		 UE_LOG(LogTemp, Warning, TEXT("%s: CapsuleComponent not found!"), *FString(__FUNCTION__));
		 return;
	}

	if (bEnableGravityAndCollision)
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Enabling Gravity and Default Collision/Movement"), *FString(__FUNCTION__));
		MoveComp->SetMovementMode(MOVE_Walking);
		MoveComp->GravityScale = 1.0f; 
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Capsule->SetCollisionResponseToAllChannels(ECR_Block);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s: Disabling Gravity and Collision Effects"), *FString(__FUNCTION__));

		MoveComp->SetMovementMode(MOVE_Flying);
		MoveComp->GravityScale = 0.0f;
		MoveComp->Velocity = FVector::ZeroVector;
		MoveComp->StopMovementImmediately();
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Capsule->SetCollisionResponseToAllChannels(ECR_Overlap);
	}
}

void ADiaBaseCharacter::OnLevelUp()
{
	if(!IsValid(LevelComponent))
	{
		return;
	}

	LevelComponent->LevelUp();
}

void ADiaBaseCharacter::PauseCurrentMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (IsValid(AnimInstance) && IsValid(CurrentMontage))
	{
		AnimInstance->Montage_Pause(CurrentMontage);
	}
}

void ADiaBaseCharacter::PauseDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (IsValid(AnimInstance) && IsValid(DieMontage))
	{
		AnimInstance->Montage_Pause(DieMontage);
	}
}

void ADiaBaseCharacter::SetTargetActor(ADiaBaseCharacter* NewTarget)
{

}

void ADiaBaseCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CurrentMontage == Montage)
	{
		CurrentMontage = nullptr;

		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &ADiaBaseCharacter::OnMontageEnded);
		}
	}
}

UAbilitySystemComponent* ADiaBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

