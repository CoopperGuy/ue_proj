// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/DiaSkillObject.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "Sound/SoundBase.h"

#include "GameplayEffect.h"
#include "DiaBaseCharacter.h"

#include "GAS/DiaGameplayTags.h"

#include "Kismet/GameplayStatics.h"
#include "Skill/DiaDamageType.h"


// Sets default values
ADiaSkillObject::ADiaSkillObject()
{
	PrimaryActorTick.bCanEverTick = true;

    // 콜리전 컴포넌트 생성 및 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_EngineTraceChannel2);
    CollisionComp->SetCollisionProfileName("Projectile");
    CollisionComp->SetGenerateOverlapEvents(true);  // Overlap 이벤트 활성화
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ADiaSkillObject::OnHit);

    RootComponent = CollisionComp;

    // 프로젝타일 메시 컴포넌트 생성 및 설정
    SkilleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    SkilleMesh->SetupAttachment(RootComponent);
    SkilleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkilleMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

    // 기본 수명 설정
    LifeSpan = 5.0f;
    InitialLifeSpan = LifeSpan;

    LagacySkillAbilityEffectComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LagacySkillAbilityEffectComp"));
    LagacySkillAbilityEffectComp->SetupAttachment(RootComponent);
    LagacySkillAbilityEffectComp->bAutoActivate = false;
    LagacySkillAbilityEffectComp->bAutoDestroy = false;
    LagacySkillAbilityEffectComp->SetRelativeLocation(FVector::ZeroVector);

    SkillAbilityEffectComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SkillAbilityEffectComp"));
    SkillAbilityEffectComp->SetupAttachment(RootComponent);
    SkillAbilityEffectComp->bAutoActivate = false;
    SkillAbilityEffectComp->SetAutoDestroy(0);
    SkillAbilityEffectComp->SetRelativeLocation(FVector::ZeroVector);
}

void ADiaSkillObject::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADiaSkillObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADiaSkillObject::OnProjectileHit(ADiaBaseCharacter* HitActor, const FHitResult& HitResult)
{
}

void ADiaSkillObject::ProcessDamage(IAbilitySystemInterface* ASCInterface, const FHitResult& HitResult)
{
    if (!(ASCInterface) || !IsValid(Owner))
    {
        return;
    }


    // GAS가 설정된 경우: GameplayEffect를 통해 대미지 적용
    if (SourceASC.IsValid() && DamageGameplayEffect)
    {
        UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent();
        if (TargetASC)
        {
            // 무적 상태 체크
            if (TargetASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Invincible)) return;

            FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
            EffectContext.AddInstigator(GetOwner(), Cast<APawn>(GetOwner()));

            FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGameplayEffect, 1.0f, EffectContext);
            if (SpecHandle.IsValid())
            {
                // SetByCaller로 기본 대미지 주입(Exec_Damage가 참조 가능)
                SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("GASData.DamageBase")), Damage);

                // 필요 시 치명타 배수 등 추가 세팅 가능
                // SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("GASData.CritMultiplier")), 1.0f);

                SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
            }
        }
    }

    // 피격 사운드 재생
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitResult.ImpactPoint);
    }
}

void ADiaSkillObject::SpawnHitEffect(const FVector& ImpactPoint, const FVector& ImpactNormal)
{
    // 피격 이펙트 생성
    if (HitEffect)
    {
        UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            HitEffect,
            ImpactPoint,
            ImpactNormal.Rotation()
        );

        if (NiagaraComp)
        {
            NiagaraComp->SetVariableFloat(FName("EffectScale"), 1.0f);
        }
    }
}