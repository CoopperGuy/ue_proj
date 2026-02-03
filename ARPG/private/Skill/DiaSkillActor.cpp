// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/DiaSkillActor.h"

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

#include "Interface/Damageable.h"

#include <AbilitySystemBlueprintLibrary.h>

#include "Kismet/GameplayStatics.h"
#include "Skill/DiaDamageType.h"

#include "DiaComponent/Skill/Executor/DiaSkillHitVariantExecutor.h"
#include "DiaComponent/DiaSkillManagerComponent.h"


// Sets default values
ADiaSkillActor::ADiaSkillActor()
{
	PrimaryActorTick.bCanEverTick = true;

    // 콜리전 컴포넌트 생성 및 설정
    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(15.0f);
    CollisionComp->SetCollisionObjectType(ECollisionChannel::ECC_EngineTraceChannel2);
    CollisionComp->SetCollisionProfileName("Projectile");
    // Skill 채널(SkillActor끼리) 충돌 무시 설정
    CollisionComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_EngineTraceChannel2, ECollisionResponse::ECR_Ignore);
    CollisionComp->SetGenerateOverlapEvents(true);  // Overlap 이벤트 활성화
    CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ADiaSkillActor::OnHit);

    RootComponent = CollisionComp;

    // 프로젝타일 메시 컴포넌트 생성 및 설정
    SkilleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkillObjectMesh"));
    SkilleMesh->SetupAttachment(RootComponent);
    SkilleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkilleMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
    SkilleMesh->SetVisibility(true);
    // 기본 수명 설정
    LifeSpan = 5.0f;
    InitialLifeSpan = LifeSpan;

    LagacySkillAbilityEffectComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("LagacySkillAbilityEffectComp"));
    LagacySkillAbilityEffectComp->SetupAttachment(RootComponent);
    LagacySkillAbilityEffectComp->bAutoActivate = false;
    LagacySkillAbilityEffectComp->bAutoDestroy = false;
    LagacySkillAbilityEffectComp->SetRelativeLocation(FVector::ZeroVector);

    SkillAbilityEffectComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SkillEffectComp"));
    SkillAbilityEffectComp->SetupAttachment(RootComponent);
    SkillAbilityEffectComp->bAutoActivate = false;
	SkillAbilityEffectComp->SetAutoDestroy(false);
    SkillAbilityEffectComp->SetRelativeLocation(FVector::ZeroVector);
}

void ADiaSkillActor::Initialize(float InDamage, AActor* InOwner, UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageEffect)
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

void ADiaSkillActor::Initialize(const FGASSkillData& SkillData, AActor* InOwner, UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageEffect)
{
	Damage = SkillData.BaseDamage;
	IntervalBetweenHits = SkillData.HitInterval;
	MaxHitCount = SkillData.HitCount;
    SetOwner(InOwner);
    // 발사체 소유자와의 충돌 방지
    if (IsValid(Owner))
    {
        CollisionComp->IgnoreActorWhenMoving(Owner, true);
    }
    SourceASC = InSourceASC;
	DamageGameplayEffect = InDamageEffect;
}

void ADiaSkillActor::InitTargetEffectHandle(const TArray<FGameplayEffectSpecHandle>& InTargetEffectHandles)
{
	TargetEffectHandles = InTargetEffectHandles;
}

void ADiaSkillActor::BeginPlay()
{
	Super::BeginPlay();

    // 메시 컴포넌트 렌더 상태 강제 업데이트 (멀티샷 시각화 문제 해결)
    if (IsValid(SkilleMesh))
    {
        SkilleMesh->SetVisibility(true);
        SkilleMesh->SetHiddenInGame(false);
        SkilleMesh->RecreateRenderState_Concurrent();
    }

    // 생성자에서 만든 컴포넌트에 나이아가라 시스템 설정
    if (IsValid(SkillAbilityEffectComp) && SkillEffect)
    {
        SkillAbilityEffectComp->SetAsset(SkillEffect);
        SkillAbilityEffectComp->SetVariableFloat(FName("EffectScale"), 1.0f);
        SkillAbilityEffectComp->Activate(true);
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillActor::BeginPlay - SkillEffect activated."));
    }
    else
    {
        bool isSkillEffectValid = IsValid(SkillEffect);
		FString NameString = isSkillEffectValid ? SkillEffect->GetName() : TEXT("None");
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillActor::BeginPlay - SkillEffect or SkillAbilityEffectComp is not valid. effect : %s"), *NameString);
    }
}

void ADiaSkillActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADiaSkillActor::OnHit(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& HitResult)
{
    if (!OtherActor)
    {
        return;
    }
    
    ADiaBaseCharacter* OwnerActor = Cast<ADiaBaseCharacter>(Owner);
    if (!IsValidTarget(OtherActor))
    {
        return;
    }

    // 다른 프로젝타일이나 스킬 액터와의 충돌 무시 (같은 스킬에서 스폰된 다른 발사체)
    if (ADiaSkillActor* OtherSkillActor = Cast<ADiaSkillActor>(OtherActor))
    {
        // 같은 소유자를 가진 다른 스킬 액터는 무시
        if (OtherSkillActor->GetOwner() == Owner)
        {
            return;
        }
    }

    // Pierce: 이미 히트한 적은 다시 히트하지 않음
    if (HitActors.Contains(OtherActor))
    {
        return;
    }

    ApplyGameplayHit(OtherActor, HitResult, OwnerActor);
}

void ADiaSkillActor::ApplyGameplayHit(AActor* OtherActor, const FHitResult& HitResult, ADiaBaseCharacter* OwnerActor)
{
    IAbilitySystemInterface* DiaOtherActor = Cast<IAbilitySystemInterface>(OtherActor);
    if (DiaOtherActor)
    {
        // 이미 히트한 적 목록에 추가
        HitActors.Add(OtherActor);

        // 데미지 처리
        ProcessDamage(DiaOtherActor, HitResult);

        // 피격 이펙트 생성
        SpawnHitEffect(HitResult.ImpactPoint, HitResult.ImpactNormal);

        // 피격 이벤트 호출
        OnSkillHit(DiaOtherActor, HitResult);

        //피격시 효과 타겟에게 적용.
        ProcessTargetEffects(DiaOtherActor);
        //타격에 성공하면 받으면 일단 타겟으로 올린다.
        //HACK
        if (IsValid(OwnerActor))
        {
            OwnerActor->SetTargetActor(Cast<ADiaBaseCharacter>(OtherActor));
        }

        // Pierce: HitExecutor 호출하여 Pierce Effect 적용
        if (IsValid(OwnerActor) && OwnerActor->GetSkillManagerComponent())
        {
            UDiaGameplayAbility* OwningAbilityPtr = OwningAbility.Get();
            if (OwningAbilityPtr)
            {
                FDiaSkillVariantContext VariantContext;
                VariantContext.HitResult = HitResult;
                VariantContext.SkillActor = this;
                VariantContext.AbilityComp = OwnerActor->GetAbilitySystemComponent();
                
                // Variants 배열 생성
                TArray<UDiaSkillVariant*> VariantsToApply;
                OwnerActor->GetSkillManagerComponent()->MakeSkillVariantsArray(OwningAbilityPtr, VariantsToApply);
                
                // HitExecutor 생성 및 실행 (Runtime 반환 버전)
                UDiaSkillHitVariantExecutor* HitExecutor = NewObject<UDiaSkillHitVariantExecutor>(OwnerActor->GetSkillManagerComponent());
                FSkillHitRuntime HitRuntime;
                HitRuntime.PierceCount = PierceCount; // 현재 PierceCount 설정
                HitExecutor->ExecuteEffect(VariantsToApply, VariantContext, OwningAbilityPtr, HitRuntime);
                
                // Runtime의 PierceCount를 SkillActor에 반영
                PierceCount = HitRuntime.PierceCount;
            }
        }

        // Pierce: PierceCount가 0 이하면 발사체 파괴
        if (PierceCount <= 0)
        {
            Destroy();
        }
    }
}

void ADiaSkillActor::OnHitDetect()
{
}

void ADiaSkillActor::OnSkillHit(IAbilitySystemInterface* HitActor, const FHitResult& HitResult)
{
}

void ADiaSkillActor::ProcessDamage(IAbilitySystemInterface* ASCInterface, const FHitResult& HitResult)
{
    if (!(ASCInterface) || !IsValid(Owner))
    {
        return;
    }


    //투사체가 맞는 시점에서 상대의 상태를 체크해야 하므로 나중에 생성하도록 설정.
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

void ADiaSkillActor::SpawnHitEffect(const FVector& ImpactPoint, const FVector& ImpactNormal)
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

void ADiaSkillActor::ProcessTargetEffects(IAbilitySystemInterface* Target)
{
    if (!SourceASC.IsValid())
        return;
    if(!TargetEffectHandles.Num())
		return;
    if(!Target)
		return;
    UAbilitySystemComponent* TargetASC = Target->GetAbilitySystemComponent();
    if (!IsValid(TargetASC))
        return;

    for(const FGameplayEffectSpecHandle& SpecHandle : TargetEffectHandles)
    {
        FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
        if (Spec)
        {
            FActiveGameplayEffectHandle ActiveHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*Spec, TargetASC);
            FGameplayTagContainer gmte;
            Spec->GetAllGrantedTags(gmte);
        }
    }
}

void ADiaSkillActor::Launch(const FVector& Direction)
{

}

bool ADiaSkillActor::IsValidTarget(AActor* OtherActor)
{
    if (!OtherActor)
    {
        return false;
    }
    
    ADiaBaseCharacter* OwnerActor = Cast<ADiaBaseCharacter>(Owner);
    
    if (!IsValid(OtherActor))
    {
        return false;
    }
    
    if (OtherActor == this || OtherActor == OwnerActor)
    {
        return false;
    }

    // 태그 검사 로직: 팀 구분용 태그만 체크 (공통 태그는 제외)
    if (IsValid(OwnerActor))
    {
        static const FName CharacterTag = FName(TEXT("Character")); // 공통 태그는 제외
        for (const FName& OwnerTag : OwnerActor->Tags)
        {
            // "Character" 태그는 모든 캐릭터에 공통이므로 제외
            if (OwnerTag == CharacterTag)
            {
                continue;
            }
            
            if (OtherActor->ActorHasTag(OwnerTag))
            {
                return false;
            }
        }
    }
    
    return true;
}