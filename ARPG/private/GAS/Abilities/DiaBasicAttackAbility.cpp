#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "DiaBaseCharacter.h"
#include "GAS/DiaAttributeSet.h"
#include "GAS/Effects/DiaGameplayEffect_Damage.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameplayEffect.h"
#include "TimerManager.h"
#include "GameFramework/DamageType.h"
#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"

UDiaBasicAttackAbility::UDiaBasicAttackAbility()
{
	// 기본 설정
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// 쿨다운 설정 (2초)
	CooldownDuration = 2.0f;
	
	// 마나 소모 (10)
	ManaCost = 10.0f;
	
	AttackRange = 200.0f;
	AttackAngle = 90.0f;
	BaseDamage = 50.0f;
}

void UDiaBasicAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("DiaBasicAttackAbility: Activating Basic Attack"));
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Basic Attack Activated!"));
	}

	// 애니메이션 재생
	if (AttackMontage)
	{
		if (ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(GetAvatarActorFromActorInfo()))
		{
			Character->PlayCharacterMontage(AttackMontage);
		}
		
		// 애니메이션 중간에 공격 실행 (0.5초 후)
		GetWorld()->GetTimerManager().SetTimer(
			AttackTimer,
			this,
			&UDiaBasicAttackAbility::PerformAttack,
			0.5f,
			false
		);
	}
	else
	{
		// 애니메이션이 없으면 즉시 공격
		PerformAttack();
	}
}

void UDiaBasicAttackAbility::PerformAttack()
{
	ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("DiaBasicAttackAbility: Performing Attack"));
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Attack Hit Check!"));
	}

	// 캐릭터 위치와 방향 가져오기
	FVector CharacterLocation = Character->GetActorLocation();
	FVector ForwardVector = Character->GetActorForwardVector();
	
	// 구체 스윕으로 적 탐지
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
	QueryParams.bTraceComplex = false;
	
	TArray<FHitResult> HitResults;
	FVector StartLocation = CharacterLocation;
	FVector EndLocation = CharacterLocation + (ForwardVector * AttackRange);
	
	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(50.0f),
		QueryParams
	);

	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				// 각도 체크
				FVector ToTarget = (HitActor->GetActorLocation() - CharacterLocation).GetSafeNormal();
				float DotProduct = FVector::DotProduct(ForwardVector, ToTarget);
				float AngleInDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
				
				if (AngleInDegrees <= AttackAngle / 2.0f)
				{
					// GAS 방식으로 대미지 적용
					if (ADiaBaseCharacter* TargetCharacter = Cast<ADiaBaseCharacter>(HitActor))
					{
						UAbilitySystemComponent* TargetASC = TargetCharacter->GetAbilitySystemComponent();
						if (TargetASC)
						{
							// 대미지 GameplayEffect 생성 및 적용
							FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
							EffectContext.AddSourceObject(Character);
							
							// 데미지 계산
							float FinalDamage = BaseDamage;
							if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
							{
								if (const UDiaAttributeSet* AttributeSet = Cast<UDiaAttributeSet>(ASC->GetAttributeSet(UDiaAttributeSet::StaticClass())))
								{
									FinalDamage += AttributeSet->GetAttackPower();
								}
							}
							
							// GameplayEffect 생성
							FGameplayEffectSpecHandle DamageSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
								UDiaGameplayEffect_Damage::StaticClass(), 1, EffectContext);
								
							if (DamageSpecHandle.IsValid())
							{
								// 대미지 값 설정
								DamageSpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), FinalDamage);
								
								// 대미지 적용
								GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
								
								UE_LOG(LogTemp, Log, TEXT("DiaBasicAttackAbility: Applied GAS damage to %s for %.1f"), *HitActor->GetName(), FinalDamage);
								
								if (GEngine)
								{
									GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
										FString::Printf(TEXT("GAS Hit %s for %.1f damage!"), *HitActor->GetName(), FinalDamage));
								}
							}
						}
					}
					else
					{
						// Non-GAS 액터에 대한 폴백 (기존 방식)
						float FinalDamage = BaseDamage;
						if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
						{
							if (const UDiaAttributeSet* AttributeSet = Cast<UDiaAttributeSet>(ASC->GetAttributeSet(UDiaAttributeSet::StaticClass())))
							{
								FinalDamage += AttributeSet->GetAttackPower();
							}
						}
						
						FPointDamageEvent PointEvent;
						PointEvent.Damage = FinalDamage;
						PointEvent.HitInfo = Hit;
						
						HitActor->TakeDamage(FinalDamage, static_cast<const FDamageEvent&>(PointEvent), Character->GetController(), Character);
						
						UE_LOG(LogTemp, Log, TEXT("DiaBasicAttackAbility: Legacy damage to %s for %.1f"), *HitActor->GetName(), FinalDamage);
						
						if (GEngine)
						{
							GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, 
								FString::Printf(TEXT("Legacy Hit %s for %.1f damage!"), *HitActor->GetName(), FinalDamage));
						}
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("DiaBasicAttackAbility: No targets hit"));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("No targets hit"));
		}
	}

	// 스킬 종료
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDiaBasicAttackAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 정리
	if (GetWorld() && AttackTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackTimer);
	}

	UE_LOG(LogTemp, Log, TEXT("DiaBasicAttackAbility: Ability Ended"));
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, TEXT("Basic Attack Ended"));
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}