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

void ADiaProjectile::OnHit(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& HitResult)
{
    ADiaBaseCharacter* OnwerActor = Cast<ADiaBaseCharacter>(Owner);
    if (!IsValid(OtherActor) || OtherActor == this || OtherActor == OnwerActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("DiaProjectile::OnHit - Invalid OtherActor or self/owner. Ignore hit."));
        return;
    }

    // 소유자와 타겟의 태그를 비교
	bool bIsOwnerCharacter = true;
    if (IsValid(OnwerActor))
    {
        // 소유자의 모든 태그에 대해 검사
        for (const FName& OwnerTag : OnwerActor->Tags)
        {
            if (!OtherActor->ActorHasTag(OwnerTag))
            {
                bIsOwnerCharacter = false;
                continue;
            }
        }
    }

    // 같은 태그를 가진 액터는 데미지를 받지 않음
    if (bIsOwnerCharacter)
    {
		UE_LOG(LogTemp, Warning, TEXT("DiaProjectile::OnHit - OtherActor shares tag with Owner. Ignore hit."));
        return;
    }


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
        OnwerActor->SetTargetActor(DiaOtherActor);
    }
    
    // 발사체 제거
    Destroy();
}

void ADiaProjectile::OnProjectileHit(ADiaBaseCharacter* HitActor, const FHitResult& HitResult)
{
	Super::OnProjectileHit(HitActor, HitResult);
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

