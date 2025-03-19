// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/DiaProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DiaBaseCharacter.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"

// Sets default values
ADiaProjectile::ADiaProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // 콜리전 컴포넌트 생성 및 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionProfileName("Projectile");
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ADiaProjectile::OnHit);
    RootComponent = CollisionComp;

    // 프로젝타일 무브먼트 컴포넌트 생성 및 설정
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = ProjectileSpeed;
    ProjectileMovement->MaxSpeed = ProjectileSpeed;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    // 프로젝타일 메시 컴포넌트 생성 및 설정
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ProjectileMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));

    // 기본 수명 설정
    projectileLifeSpan = 5.0f;
    InitialLifeSpan = projectileLifeSpan;
}

// Called when the game starts or when spawned
void ADiaProjectile::BeginPlay()
{
	Super::BeginPlay();
	
    // 액터의 수명 설정
    SetLifeSpan(projectileLifeSpan);
}

// Called every frame
void ADiaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADiaProjectile::Initialize(float InDamage, AActor* InOwner)
{
    Damage = InDamage;
    ProjectileOwner = InOwner;
    SetOwner(InOwner);
    
    // 발사체 소유자와의 충돌 방지
    if (IsValid(ProjectileOwner))
    {
        CollisionComp->IgnoreActorWhenMoving(ProjectileOwner, true);
    }
}

void ADiaProjectile::OnHit(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& HitResult)
{
    if (OtherActor && OtherActor != this && OtherActor != ProjectileOwner)
    {
        // 플레이어 태그가 있는지 확인
        if (OtherActor->Tags.Contains(FName("Player")))
        {
            // 플레이어에게는 데미지를 주지 않음
            return;
        }
        
        // 데미지 처리
        ProcessDamage(OtherActor, HitResult);
        
        // 피격 이펙트 생성
        SpawnHitEffect(HitResult.ImpactPoint, HitResult.ImpactNormal);
        
        // 피격 이벤트 호출
        OnProjectileHit(OtherActor, HitResult);
        
        // 발사체 제거
        Destroy();
    }
}

void ADiaProjectile::OnProjectileHit(AActor* HitActor, const FHitResult& HitResult)
{
}

void ADiaProjectile::ProcessDamage(AActor* Target, const FHitResult& HitResult)
{
    if (!IsValid(Target) || !IsValid(ProjectileOwner))
    {
        return;
    }
    
    // 데미지 적용
    FDamageEvent DamageEvent;
    float AppliedDamage = Target->TakeDamage(Damage, DamageEvent, 
                                            ProjectileOwner->GetInstigatorController(), 
                                            ProjectileOwner);
    
    // 데미지 적용 로그
    UE_LOG(LogTemp, Log, TEXT("Projectile hit %s for %.2f damage"), 
           *Target->GetName(), AppliedDamage);
    
    // 피격 사운드 재생
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitResult.ImpactPoint);
    }
}

void ADiaProjectile::SpawnHitEffect(const FVector& ImpactPoint, const FVector& ImpactNormal)
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
            NiagaraComp->SetNiagaraVariableFloat(FString("EffectScale"), 1.0f);
        }
    }
}

