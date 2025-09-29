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
#include "DiaComponent/DiaCombatComponent.h"

#include "Engine/DamageEvents.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "GAS/Effects/DiaGameplayEffect_Damage.h"
#include "GameplayEffectTypes.h"

// Sets default values
ADiaProjectile::ADiaProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // 콜리전 컴포넌트 생성 및 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_EngineTraceChannel2);
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

    // 속도 보간 기본값
    bUseSpeedInterpolation = false;
    //TargetSpeed = 1000.0f;
    //SpeedInterpRate = 2.0f;
    //MinSpeed = 100.0f;
    //Damage = 10.0f;
	//DamageType = UDiaDamageType::StaticClass();
}
void ADiaProjectile::Launch(const FVector& Direction)
{
    if (!ProjectileMovement) return;
    const FVector LaunchVelocity = Direction.GetSafeNormal() * ProjectileSpeed;
    ProjectileMovement->Velocity = LaunchVelocity;
}


// Called when the game starts or when spawned
void ADiaProjectile::BeginPlay()
{
	Super::BeginPlay();
	
    // 액터의 수명 설정
    SetLifeSpan(projectileLifeSpan);

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
            UE_LOG(LogTemp, Warning, TEXT("%s: LegacySkillEffect 가 설정되지 않았습니다."), *GetName());
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
            UE_LOG(LogTemp, Warning, TEXT("%s: SkillEffect 가 설정되지 않았습니다."), *GetName());
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
    Damage = InDamage;
    SetOwner(InOwner);

    // 발사체 소유자와의 충돌 방지
    if (IsValid(Owner))
    {
        CollisionComp->IgnoreActorWhenMoving(Owner, true);
    }

	SourceASC = InSourceASC;
    DamageGameplayEffect = InDamageEffect;
}

void ADiaProjectile::Initialize(float InDamage, AActor* InOwner)
{
    Damage = InDamage;
    SetOwner(InOwner);
    
    // 발사체 소유자와의 충돌 방지
    if (IsValid(Owner))
    {
        CollisionComp->IgnoreActorWhenMoving(Owner, true);
    }
}

void ADiaProjectile::OnHit(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& HitResult)
{
    if (!IsValid(OtherActor) || OtherActor == this || OtherActor == Owner)
    {
        return;
    }

    // 소유자와 타겟의 태그를 비교
	bool bIsOwnerCharacter = false;
	bool bIsPlayerCharacter = false;
    if (IsValid(Owner))
    {
        // 소유자의 모든 태그에 대해 검사
        for (const FName& OwnerTag : Owner->Tags)
        {
            //character 태그에 대한 체크는 넘긴다.
            if (OwnerTag == FName(TEXT("Character")))
            {
                bIsPlayerCharacter = true;
				continue;
            }

            //같은 태크가 있으면 피격 x
            if (OtherActor->Tags.Contains(OwnerTag))
            {
                bIsOwnerCharacter = true;
                break;
            }
        }
    }

    // 같은 태그를 가진 액터는 데미지를 받지 않음
    if (bIsOwnerCharacter)    return;


    // 데미지 처리
    ADiaBaseCharacter* DiaOtherActor = Cast<ADiaBaseCharacter>(OtherActor);
    if (IsValid(DiaOtherActor))
    {
        // 데미지 처리
        ProcessDamage(DiaOtherActor, HitResult);
        
        // 피격 이펙트 생성
        SpawnHitEffect(HitResult.ImpactPoint, HitResult.ImpactNormal);
        
        // 피격 이벤트 호출
        OnProjectileHit(DiaOtherActor, HitResult);

        //타격에 성공하면 받으면 일단 타겟으로 올린다.
        //HACK
        Cast<ADiaBaseCharacter>(Owner)->SetTargetActor(DiaOtherActor);
    }
    
    // 발사체 제거
    Destroy();
}

void ADiaProjectile::OnProjectileHit(ADiaBaseCharacter* HitActor, const FHitResult& HitResult)
{
}

void ADiaProjectile::ProcessDamage(ADiaBaseCharacter* Target, const FHitResult& HitResult)
{
    if (!IsValid(Target) || !IsValid(Owner))
    {
        return;
    }
    
    UDiaCombatComponent* DiaCombatComp = Target->GetComponentByClass<UDiaCombatComponent>();

    if (!DiaCombatComp)
    {
        return;
    }
    // GAS가 설정된 경우: GameplayEffect를 통해 대미지 적용
    if (SourceASC.IsValid() && DamageGameplayEffect)
    {
        UAbilitySystemComponent* TargetASC = Target->GetComponentByClass<UAbilitySystemComponent>();
        if (TargetASC)
        {
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
            NiagaraComp->SetVariableFloat(FName("EffectScale"), 1.0f);
        }
    }
}

