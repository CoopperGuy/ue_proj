// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h" 
#include "Types/DiaGASSkillData.h"  
#include "DiaSkillActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;
class UPrimitiveComponent;
class IAbilitySystemInterface;
class ADiaBaseCharacter;
class UDiaDamageType;
class UStaticMeshComponent;
class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
class UParticleSystem;
class UParticleSystemComponent;

UCLASS()
class ARPG_API ADiaSkillActor : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
public:	
    ADiaSkillActor();

	// Called every frame
	virtual void Initialize(float InDamage, AActor* InOwner, UAbilitySystemComponent* InSourceASC = nullptr, TSubclassOf<UGameplayEffect> InDamageEffect = nullptr);
    virtual void Initialize(const FGASSkillData& SkillData, AActor* InOwner, UAbilitySystemComponent* InSourceASC = nullptr, TSubclassOf<ADiaSkillActor> InDamageEffect = nullptr);
	void InitTargetEffectHandle(const TArray<FGameplayEffectSpecHandle>& InTargetEffectHandles);

    UFUNCTION()
    virtual void OnHit(UPrimitiveComponent* OverlappedComponent, 
        AActor* OtherActor, UPrimitiveComponent* OtherComp, 
        int32 OtherBodyIndex, bool bFromSweep, 
        const FHitResult& HitResult);

    void ApplyGameplayHit(AActor* OtherActor, const FHitResult& HitResult, ADiaBaseCharacter* OnwerActor);

	UFUNCTION()
	void OnHitDetect();

    virtual void OnSkillHit(IAbilitySystemInterface* HitActor, const FHitResult& HitResult);
    
    virtual void ProcessDamage(IAbilitySystemInterface* Target, const FHitResult& HitResult);
    
    void SpawnHitEffect(const FVector& ImpactPoint, const FVector& ImpactNormal);

	void ProcessTargetEffects(IAbilitySystemInterface* Target);

    virtual void Launch(const FVector& Direction);
protected:
    bool IsValidTarget(AActor* OtherActor);
protected:    
    // 발사체 메시 컴포넌트
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* SkilleMesh;

    // 발사체 충돌 컴포넌트
    UPROPERTY(VisibleAnywhere)
    USphereComponent* CollisionComp;

    // 데미지 값
    UPROPERTY(EditDefaultsOnly, Category = "Skill|Combat")
    float Damage;

    // 발사체 수명
    UPROPERTY(EditAnywhere, Category = "Skill")
    float LifeSpan;
    
    // 피격 이펙트
    UPROPERTY(EditDefaultsOnly, Category = "Skill|Effects")
    UNiagaraSystem* HitEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* SkillEffect;

    UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Effects")
	UNiagaraComponent* SkillAbilityEffectComp;

    // 피격 사운드
    UPROPERTY(EditDefaultsOnly, Category = "Skill|Effects")
    USoundBase* HitSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
    TSubclassOf<UDiaDamageType> DamageType{nullptr};


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UParticleSystemComponent* LagacySkillAbilityEffectComp;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UParticleSystem* LegacySkillEffect;

    // GAS 관련 변수
    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> SourceASC;

    UPROPERTY(EditDefaultsOnly, Category = "Skill|GAS")
    TSubclassOf<UGameplayEffect> DamageGameplayEffect;

    UPROPERTY()
	TArray<FGameplayEffectSpecHandle> TargetEffectHandles;

    UPROPERTY()
	FTimerHandle LifeSpanTimerHandle;
	int32 HitCount = 0;
	int32 MaxHitCount = 1;
	double IntervalBetweenHits = 0.;
};

