// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/DiaDamageCalculator.h"

#include "DiaBaseCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Controller.h"
//
//FDamageResult UDiaDamageCalculator::CalculateDamage(
//    float BaseDamage,
//    AActor* DamageDealer,
//    AActor* DamageReceiver,
//    const UDiaDamageType* DamageType,
//    float CriticalChance)
//{
//    FDamageResult Result;
//    
//    if (!IsValid(DamageDealer) || !IsValid(DamageReceiver) || !IsValid(DamageType))
//    {
//        Result.FinalDamage = BaseDamage;
//        return Result;
//    }
//    
//    // 공격자 전투 컴포넌트 가져오기
//    ADiaBaseCharacter* AttackerCharacter = Cast<ADiaBaseCharacter>(DamageDealer);
//    UDiaCombatComponent* AttackerCombat = IsValid(AttackerCharacter) 
//        ? AttackerCharacter->FindComponentByClass<UDiaCombatComponent>() 
//        : nullptr;
//    UDiaStatComponent* AttackerStatCombat = IsValid(AttackerCharacter)
//        ? AttackerCharacter->FindComponentByClass<UDiaStatComponent>()
//        : nullptr;
//
//    // 방어자 전투 컴포넌트 가져오기
//    ADiaBaseCharacter* DefenderCharacter = Cast<ADiaBaseCharacter>(DamageReceiver);
//    UDiaCombatComponent* DefenderCombat = IsValid(DefenderCharacter) 
//        ? DefenderCharacter->FindComponentByClass<UDiaCombatComponent>() 
//        : nullptr;
//    UDiaStatComponent* DefenderStatCombat = IsValid(DefenderCharacter)
//        ? DefenderCharacter->FindComponentByClass<UDiaStatComponent>()
//        : nullptr;
//
//    // 기본 데미지 설정
//    float Damage = BaseDamage;
//    
//    // 공격자 스탯 적용
//    if (IsValid(AttackerStatCombat))
//    {
//        // 공격력에 따른 데미지 증가
//        Damage *= (1.0f + AttackerStatCombat->GetAttackPower() / 100.0f);
//    }
//    
//    // 크리티컬 히트 계산
//    Result.bIsCritical = RollForCritical(CriticalChance);
//    if (Result.bIsCritical)
//    {
//        Damage *= DamageType->CriticalDamageMultiplier;
//    }
//    
//    // 방어자의 방어력 적용
//    if (IsValid(DefenderCombat))
//    {
//        // 회피 확인
//        float EvasionChance = 0.05f; // 기본 5% 회피율
//        Result.bIsEvaded = RollForEvasion(EvasionChance);
//        if (Result.bIsEvaded)
//        {
//            Result.FinalDamage = 0.0f;
//            return Result;
//        }
//        
//        // 블록 확인
//        float BlockChance = 0.1f; // 기본 10% 블록율
//        Result.bIsBlocked = RollForBlock(BlockChance);
//        if (Result.bIsBlocked)
//        {
//            Damage *= 0.5f; // 블록 시 50% 데미지 감소
//        }
//        
//        // 방어력 적용
//        float Defense = 10.0f; // 기본 방어력 값
//        Damage = ApplyDefense(Damage, Defense, DamageType->ArmorPenetration);
//    }
//    
//    // 속성 데미지 계산
//    Result.ElementType = DamageType->ElementType;
//    
//    // 최종 데미지 설정
//    Result.FinalDamage = FMath::Max(1.0f, Damage); // 최소 1 데미지 보장
//    
//    // 상태 이상 효과 적용
//    if (RollForStatusEffect(DamageType->StatusEffectChance))
//    {
//        UDiaStatusEffectComponent* StatusEffectComp = 
//            DefenderCharacter->FindComponentByClass<UDiaStatusEffectComponent>();
//        
//        if (IsValid(StatusEffectComp))
//        {
//            for (TSubclassOf<UDiaStatusEffect> EffectClass : DamageType->StatusEffects)
//            {
//                UDiaStatusEffect* NewEffect = StatusEffectComp->AddStatusEffect(EffectClass, -1.0f, -1.0f, DamageDealer);
//                if (IsValid(NewEffect))
//                {
//                    Result.AppliedStatusEffects.Add(NewEffect);
//                }
//            }
//        }
//    }
//    
//    return Result;
//}
//
//float UDiaDamageCalculator::ApplyDamage(
//    AActor* DamageDealer,
//    AActor* DamageReceiver,
//    float BaseDamage,
//    TSubclassOf<UDiaDamageType> DamageTypeClass,
//    float CriticalChance)
//{
//    if (!IsValid(DamageDealer) || !IsValid(DamageReceiver) || !DamageTypeClass)
//    {
//        return 0.0f;
//    }
//    
//    // 데미지 타입 인스턴스 생성
//    UDiaDamageType* DamageType = NewObject<UDiaDamageType>(GetTransientPackage(), DamageTypeClass);
//    if (!IsValid(DamageType))
//    {
//        return 0.0f;
//    }
//    
//    // 데미지 계산
//    FDamageResult DamageResult = CalculateDamage(BaseDamage, DamageDealer, DamageReceiver, DamageType, CriticalChance);
//    
//    // 넉백 적용
//    if (DamageType->KnockbackForce > 0.0f)
//    {
//        ApplyKnockback(DamageReceiver, DamageDealer, DamageType->KnockbackForce);
//    }
//    
//    // 데미지 적용 - FPointDamageEvent 사용 (더 안전함)
//    FPointDamageEvent DamageEvent;
//    DamageEvent.DamageTypeClass = DamageTypeClass;
//    DamageEvent.HitInfo = FHitResult();  // 기본 히트 정보
//    DamageEvent.ShotDirection = (DamageReceiver->GetActorLocation() - DamageDealer->GetActorLocation()).GetSafeNormal();
//    DamageEvent.Damage = DamageResult.FinalDamage;
//    
//    float ActualDamage = DamageReceiver->TakeDamage(
//        DamageResult.FinalDamage, 
//        DamageEvent, 
//        DamageDealer->GetInstigatorController(), 
//        DamageDealer
//    );
//    
//    // 디버그 로그
//    UE_LOG(LogTemp, Log, TEXT("Damage Applied: %.2f (Base: %.2f, Critical: %s, Blocked: %s)"),
//           ActualDamage, BaseDamage, 
//           DamageResult.bIsCritical ? TEXT("Yes") : TEXT("No"),
//           DamageResult.bIsBlocked ? TEXT("Yes") : TEXT("No"));
//    
//    return ActualDamage;
//}
//
//float UDiaDamageCalculator::CalculateElementalMultiplier(EDamageElementType AttackElement, EDamageElementType DefenderElement)
//{
//    // 기본적으로 속성 상성에 따라 배율 결정
//    
//    // 같은 속성이면 약간 저항
//    if (AttackElement == DefenderElement)
//    {
//        return 0.75f;
//    }
//    
//    // 기본 배율
//    return 1.0f;
//}
//
//void UDiaDamageCalculator::ApplyKnockback(AActor* Target, AActor* Source, float KnockbackForce)
//{
//    if (!IsValid(Target) || !IsValid(Source))
//    {
//        return;
//    }
//    
//    // 넉백 방향 계산
//    FVector KnockbackDirection = Target->GetActorLocation() - Source->GetActorLocation();
//    KnockbackDirection.Z = 0.0f; // 수평 방향만 고려
//    
//    if (!KnockbackDirection.Normalize())
//    {
//        return; // 방향 정규화 실패
//    }
//    
//    // 캐릭터인 경우 움직임 컴포넌트를 통해 적용
//    ACharacter* TargetCharacter = Cast<ACharacter>(Target);
//    if (IsValid(TargetCharacter))
//    {
//        UCharacterMovementComponent* MovementComp = TargetCharacter->GetCharacterMovement();
//        if (IsValid(MovementComp))
//        {
//            // 현재 속도에 넉백 추가
//            FVector Impulse = KnockbackDirection * KnockbackForce;
//            MovementComp->AddImpulse(Impulse, true);
//        }
//    }
//    else
//    {
//        // 일반 액터인 경우 물리 컴포넌트를 찾아 적용
//        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Target->GetRootComponent());
//        if (IsValid(PrimComp) && PrimComp->IsSimulatingPhysics())
//        {
//            FVector Impulse = KnockbackDirection * KnockbackForce;
//            PrimComp->AddImpulse(Impulse, NAME_None, true);
//        }
//    }
//}
//
//float UDiaDamageCalculator::ApplyDefense(float Damage, float Defense, float ArmorPenetration)
//{
//    // 방어율 계산 (0-100%)
//    float DamageReduction = (Defense * (1.0f - ArmorPenetration)) / (50.0f + Defense);
//    
//    // 최대 75%까지 방어 가능
//    DamageReduction = FMath::Clamp(DamageReduction, 0.0f, 0.75f);
//    
//    // 방어력 적용 데미지 계산
//    return Damage * (1.0f - DamageReduction);
//}
//
////랜덤 확률 
//bool UDiaDamageCalculator::RollForCritical(float CriticalChance)
//{
//    return FMath::FRand() < CriticalChance;
//}
//
//bool UDiaDamageCalculator::RollForEvasion(float EvasionChance)
//{
//    return FMath::FRand() < EvasionChance;
//}
//
//bool UDiaDamageCalculator::RollForBlock(float BlockChance)
//{
//    return FMath::FRand() < BlockChance;
//}
//
//bool UDiaDamageCalculator::RollForStatusEffect(float StatusEffectChance)
//{
//    return FMath::FRand() < StatusEffectChance;
//}
//
//


