// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/DiaProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "DiaBaseCharacter.h"
#include "Character/DiaCharacter.h"

#include "Engine/DamageEvents.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "GAS/Effects/DiaGameplayEffect_Damage.h"
#include "GAS/DiaGameplayTags.h"
#include "GameplayEffectTypes.h"

// Sets default values
ADiaProjectile::ADiaProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    //충돌 타입 설정.
    CollisionComp->SetCollisionProfileName("Projectile");

    // 프로젝타일 무브먼트 컴포넌트 생성 및 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = ProjectileSpeed;
    ProjectileMovement->MaxSpeed = ProjectileSpeed;
	// bInitialVelocityInLocalSpace 이게 , 월드 좌표계 기준으로 회전하는거라서 false 로 해야 발사체가 원래 방향을 유지함
    // true 면 로컬임.
	ProjectileMovement->bInitialVelocityInLocalSpace = false;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    // 속도 보간 기본값
    bUseSpeedInterpolation = false;
    //TargetSpeed = 1000.0f;
    //SpeedInterpRate = 2.0f;
    //MinSpeed = 100.0f;
    //Damage = 10.0f;
	//DamageType = UDiaDamageType::StaticClass();

    SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}
void ADiaProjectile::Launch(const FVector& Direction)
{
    if (!IsValid(ProjectileMovement)) return;
    const FVector LaunchVelocity = Direction.GetSafeNormal() * ProjectileSpeed;
    ProjectileMovement->Velocity = LaunchVelocity;
    ProjectileMovement->Activate(true);
}

// Called when the game starts or when spawned
void ADiaProjectile::BeginPlay()
{
	Super::BeginPlay();

    // 액터의 수명 설정
    SetLifeSpan(LifeSpan);

    // 효과 자산 확인 및 활성화
    if (LagacySkillAbilityEffectComp)
    {
        if (LegacySkillEffect)
        {
            LagacySkillAbilityEffectComp->SetTemplate(LegacySkillEffect);
            LagacySkillAbilityEffectComp->Activate(true);
        }
        else
        {
            //UE_LOG(LogTemp, Warning, TEXT("%s: LegacySkillEffect 가 설정되지 않았습니다."), *GetName());
        }
    }

    if (SkillAbilityEffectComp)
    {
        if (SkillEffect)
        {
            SkillAbilityEffectComp->SetAsset(SkillEffect);
            SkillAbilityEffectComp->Activate(true);
        }
        else
        {
            //UE_LOG(LogTemp, Warning, TEXT("%s: SkillEffect 가 설정되지 않았습니다."), *GetName());
        }
    }
}

void ADiaProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

// Called every frame
void ADiaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADiaProjectile::Initialize(float InDamage, AActor* InOwner, UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageEffect)
{
    Super::Initialize(InDamage, InOwner, InSourceASC, InDamageEffect);
}

void ADiaProjectile::OnHit(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& HitResult)
{
    ADiaBaseCharacter* OnwerActor = Cast<ADiaBaseCharacter>(Owner);
    if (Owner)
    {
		UE_LOG(LogTemp, Warning, TEXT("ADiaSkillActor::OnHit - Owner : %s"), *Owner->GetName());
    }
    if (OnwerActor)
    {
		UE_LOG(LogTemp, Warning, TEXT("ADiaSkillActor::OnHit - OnwerActor : %s"), *OnwerActor->GetName());
    }
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("ADiaSkillActor::OnHit - OnwerActor is null."));
    }
    if (!IsValidTarget(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("ADiaSkillActor::OnHit - Invalid OtherActor or self/owner. Ignore hit."));
        return;
    }

    ApplyGameplayHit(OtherActor, HitResult, OnwerActor);
    // 발사체 제거
    Destroy();
}

void ADiaProjectile::OnSkillHit(IAbilitySystemInterface* HitActor, const FHitResult& HitResult)
{
	Super::OnSkillHit(HitActor, HitResult);
}

void ADiaProjectile::ProcessDamage(IAbilitySystemInterface* ASCInterface, const FHitResult& HitResult)
{
    if (!(ASCInterface) || !IsValid(Owner))
    {
        return;
    }

	Super::ProcessDamage(ASCInterface, HitResult);
}

void ADiaProjectile::SpawnHitEffect(const FVector& ImpactPoint, const FVector& ImpactNormal)
{
	Super::SpawnHitEffect(ImpactPoint, ImpactNormal);
}

