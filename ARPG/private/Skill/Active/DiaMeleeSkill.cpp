// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/Active/DiaMeleeSkill.h"
#include "Engine/DamageEvents.h"

#include "DiaComponent/DiaCombatComponent.h"

#include "DiaBaseCharacter.h"
#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "DrawDebugHelpers.h"

ADiaMeleeSkill::ADiaMeleeSkill()
{
    // 기본 설정
}

void ADiaMeleeSkill::ExecuteSkill()
{
    // 히트 액터 목록 초기화
    HitActors.Empty();
    
    Super::ExecuteSkill();
}

void ADiaMeleeSkill::EndSkill()
{
    Super::EndSkill();
}

void ADiaMeleeSkill::OnSkillStart()
{
    Super::OnSkillStart();
    
    // 몽타주가 없으면 즉시 피격 판정 수행
    if (!IsValid(SkillMontage))
    {
        PerformHitDetection();
        EndSkill();
    }
    // 몽타주가 있으면 애니메이션 노티파이에서 OnMeleeHitFrame이 호출됨
}

void ADiaMeleeSkill::OnSkillEnd()
{
    Super::OnSkillEnd();
}

void ADiaMeleeSkill::OnMeleeHitFrame()
{
    // 애니메이션 노티파이에서 호출될 함수
    PerformHitDetection();
}

void ADiaMeleeSkill::PerformHitDetection()
{
    if (!IsValid(SkillOwner))
    {
        return;
    }

    // 공격 원점 계산
    FVector AttackOrigin = SkillOwner->GetActorLocation() + 
                          SkillOwner->GetActorRotation().RotateVector(AttackOffset);
    FVector ForwardVector = SkillOwner->GetActorForwardVector();
    
    // 충돌 검출 파라미터
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    
    TArray<AActor*> ActorsToIgnore = { SkillOwner, this };
    TArray<AActor*> OverlappedActors;
    
    // 구체 오버랩 검출
    bool bHasOverlaps = UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        AttackOrigin,
        AttackRange,
        ObjectTypes,
        nullptr,
        ActorsToIgnore,
        OverlappedActors
    );
    
    // 디버그 시각화
    if (bShowDebugShape)
    {
        DrawDebugSphere(GetWorld(), AttackOrigin, AttackRange, 12, FColor::Red, false, 2.0f);
    }
    
    // 검출된 액터 필터링 및 데미지 적용
    for (AActor* HitActor : OverlappedActors)
    {
        // 이미 데미지를 적용한 액터 스킵
        if (HitActors.Contains(HitActor))
        {
            continue;
        }
        
        // 플레이어는 공격하지 않음
        if (HitActor->Tags.Contains(FName("Player")))
        {
            continue;
        }
        
        // 각도 체크
        FVector DirectionToTarget = (HitActor->GetActorLocation() - AttackOrigin).GetSafeNormal();
        float DotProduct = FVector::DotProduct(ForwardVector, DirectionToTarget);
        float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));
        
        // 공격 각도 내에 있는지 확인
        if (AngleDeg <= AttackAngle / 2.0f)
        {
            // 히트 목록에 추가
            HitActors.Add(HitActor);
            
            // 데미지 적용
            ApplyDamage(HitActor);
            
            // 디버그 라인
            if (bShowDebugShape)
            {
                DrawDebugLine(GetWorld(), AttackOrigin, HitActor->GetActorLocation(), 
                             FColor::Green, false, 2.0f, 0, 2.0f);
            }
        }
        else if (bShowDebugShape)
        {
            // 각도 밖 디버그 표시
            DrawDebugLine(GetWorld(), AttackOrigin, HitActor->GetActorLocation(), 
                         FColor::Yellow, false, 2.0f, 0, 1.0f);
        }
    }
}

void ADiaMeleeSkill::ApplyDamage(AActor* Target)
{
    if (!IsValid(Target) || !IsValid(SkillOwner))
    {
        return;
    }
    
    // 전투 컴포넌트 가져오기
    UDiaCombatComponent* CombatComp = SkillOwner->GetComponentByClass<UDiaCombatComponent>();
    if (!IsValid(CombatComp))
    {
        return;
    }
    
    // 데미지 적용
    float FinalDamage = GetDamage();
    CombatComp->ApplyDamage(Target, FinalDamage);
    
    // 피격 이펙트 생성
    if (HitEffect)
    {
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            HitEffect,
            Target->GetActorLocation(),
            FRotator::ZeroRotator
        );
    }
    
    // 피격 사운드 재생
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            HitSound,
            Target->GetActorLocation()
        );
    }
    
    // 상태 이상 효과 적용
    ApplyStatusEffectsToTarget(Target);
}




