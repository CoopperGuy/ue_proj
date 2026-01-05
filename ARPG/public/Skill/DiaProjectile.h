// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Skill/DiaSkillType.h"
#include "GameplayEffectTypes.h"
#include "Skill/DiaSkillActor.h"
#include "DiaProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class USoundBase;
class UNiagaraSystem;
class ADiaBaseCharacter;
class UDiaDamageType;
class UGameplayEffect;
class UAbilitySystemComponent;
class UNiagaraComponent;
class UParticleSystemComponent;
class IAbilitySystemInterface;
UCLASS()
class ARPG_API ADiaProjectile : public ADiaSkillActor
{
	GENERATED_BODY()
	
public:	
	ADiaProjectile();

	// 발사 방향으로 속도를 설정하여 발사
	void Launch(const FVector& Direction);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	virtual void Tick(float DeltaTime) override;

	void Initialize(float InDamage, AActor* InOwner, UAbilitySystemComponent* InSourceASC = nullptr, TSubclassOf<UGameplayEffect> InDamageEffect = nullptr);
	
    virtual void OnHit(UPrimitiveComponent* OverlappedComponent, 
        AActor* OtherActor, UPrimitiveComponent* OtherComp, 
        int32 OtherBodyIndex, bool bFromSweep, 
        const FHitResult& HitResult);

    // 발사체가 타겟에 적중했을 때 실행될 함수
    virtual void OnSkillHit(IAbilitySystemInterface* HitActor, const FHitResult& HitResult);
    
    // 실제 데미지를 처리하는 함수
    virtual void ProcessDamage(IAbilitySystemInterface* Target, const FHitResult& HitResult);
    
    // 피격 이펙트 생성
    void SpawnHitEffect(const FVector& ImpactPoint, const FVector& ImpactNormal);

protected:    
    // 발사체 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Movement")
    float ProjectileSpeed = 3000.0f;

    // 발사체 이동 컴포넌트
    UPROPERTY(VisibleAnywhere)
    UProjectileMovementComponent* ProjectileMovement;
        
    // 속도 보간 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Movement")
    bool bUseSpeedInterpolation = false;

    // 목표 속도 (보간 시 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Movement", meta = (EditCondition = "bUseSpeedInterpolation"))
    float TargetSpeed = 1000.0f;

    // 속도 보간 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Movement", meta = (EditCondition = "bUseSpeedInterpolation"))
    float SpeedInterpRate = 2.0f;

    // 최소 속도 제한
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Movement", meta = (EditCondition = "bUseSpeedInterpolation"))
    float MinSpeed = 100.0f;
};
