// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/DiaSkillBase.h"
#include "DiaBaseCharacter.h"

// Sets default values
ADiaSkillBase::ADiaSkillBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bIsOnCooldown = false;
}

void ADiaSkillBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADiaSkillBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADiaSkillBase::InitializeSkill(AActor* InOwner)
{
	SkillOwner = Cast<ADiaBaseCharacter>(InOwner);
}

bool ADiaSkillBase::CanExecuteSkill() const
{
	return !bIsOnCooldown && !bIsExecuting && SkillOwner != nullptr;
}

void ADiaSkillBase::ExecuteSkill()
{
	if (CanExecuteSkill())
	{
		bIsExecuting = true;
		OnSkillStart();
	}
}

void ADiaSkillBase::EndSkill()
{
	OnSkillEnd();
	
	// 쿨다운 시작
	bIsOnCooldown = true;
	FTimerHandle CooldownTimer;
	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimer, 
		[this]() { bIsOnCooldown = false; }, 
		SkillData.Cooldown, 
		false
	);
}

void ADiaSkillBase::OnSkillStart()
{
	// 기본 구현
	if (IsValid(SkillOwner))
	{

		// 이미 재생 중인 몽타주가 있다면 중지
		if (SkillOwner->IsPlayingMontage(SkillMontage))
		{
			SkillOwner->StopAnimMontage(SkillMontage);
		}

		if (IsValid(SkillMontage))
		{
			//UE_LOG(LogTemp, Warning, TEXT("몽타주 유효함: %s"), *SkillMontage->GetName());
			// 몽타주를 직접 재생
			ACharacter* ownerCharacter = Cast<ACharacter>(GetOwner());
			if (IsValid(ownerCharacter))
			{
				UAnimInstance* animInstance = ownerCharacter->GetMesh()->GetAnimInstance();
				if (IsValid(animInstance))
				{
					float duration = animInstance->Montage_Play(SkillMontage, 1.0f);
					//UE_LOG(LogTemp, Warning, TEXT("몽타주 재생 시작, 길이: %f"), duration);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("몽타주가 유효하지 않음"));
		}
	}
}

void ADiaSkillBase::OnSkillEnd()
{
	bIsExecuting = false;
	
	// 쿨다운 시작
	bIsOnCooldown = true;
	FTimerHandle CooldownTimer;
	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimer, 
		[this]() { bIsOnCooldown = false; }, 
		SkillData.Cooldown,
		false
	);
}

void ADiaSkillBase::InitializeWithData(const FSkillData* InSkillData)
{
	if (InSkillData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Null SkillData provided to InitializeWithData"));
		return;
	}
	
	// 스킬 데이터 전체를 복사
	SkillData = *InSkillData;

#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
	UE_LOG(LogTemp, Log, TEXT("스킬 초기화 - 이름: %s"), *SkillData.SkillName.ToString());
	UE_LOG(LogTemp, Log, TEXT("스킬 설명: %s"), *SkillData.SkillDescription.ToString());
	UE_LOG(LogTemp, Log, TEXT("쿨다운: %.2f"), SkillData.Cooldown);
	UE_LOG(LogTemp, Log, TEXT("마나 소모량: %.2f"), SkillData.ManaCost);
	UE_LOG(LogTemp, Log, TEXT("데미지: %.2f"), SkillData.Damage);
#endif
}

void ADiaSkillBase::ApplyStatusEffectsToTarget(AActor* Target)
{
	if (!IsValid(Target) || StatusEffects.Num() == 0)
	{
		return;
	}
	
	// 적용 확률 체크
	if (FMath::FRand() > StatusEffectChance)
	{
		return;
	}
	
	// 타겟의 상태 이상 효과 컴포넌트 가져오기
	UDiaStatusEffectComponent* StatusEffectComp = Target->FindComponentByClass<UDiaStatusEffectComponent>();
	if (!IsValid(StatusEffectComp))
	{
		return;
	}
	
	// 각 상태 이상 효과 적용
	for (TSubclassOf<UDiaStatusEffect> EffectClass : StatusEffects)
	{
		if (EffectClass)
		{
			StatusEffectComp->AddStatusEffect(EffectClass, -1.0f, -1.0f, SkillOwner);
		}
	}
}

