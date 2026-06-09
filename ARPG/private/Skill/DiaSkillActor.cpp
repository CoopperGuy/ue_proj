// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill/DiaSkillActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"

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
#include "Engine/World.h"
#include "TimerManager.h"
#include "Skill/DiaDamageType.h"

#include "Engine/OverlapResult.h"

#include "DiaComponent/DiaSkillManagerComponent.h"
#include "Logging/ARPGLogChannels.h"


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

    Tags.Add(InOwner->Tags[1]);
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
    Tags.Add(InOwner->Tags[1]);
}

void ADiaSkillActor::InitTargetEffectHandle(const TArray<FGameplayEffectSpecHandle>& InTargetEffectHandles)
{
	TargetEffectHandles = InTargetEffectHandles;
}

void ADiaSkillActor::AddIgnoredHitActors(const TArray<AActor*>& InIgnoredActors)
{
    for (AActor* IgnoredActor : InIgnoredActors)
    {
        if (!IsValid(IgnoredActor))
        {
            continue;
        }

        HitActors.Add(IgnoredActor);

        if (CollisionComp)
        {
            CollisionComp->IgnoreActorWhenMoving(IgnoredActor, true);
        }
    }
}

void ADiaSkillActor::ApplySpawnRuntimeParams(const FDiaSkillRuntimeParams& InParams)
{
    RuntimeParams = InParams;
    
    const FDiaGameplayTags& GameplayTags = FDiaGameplayTags::Get();
    DamageMultiplier = RuntimeParams.GetMagnitude(GameplayTags.GASData_Variant_DamageMultiplier, 1.f);
    PierceCount = FMath::Max(1, RuntimeParams.GetInt(GameplayTags.GASData_Pierce, 1));
    
    float SpawnDamageMultiplier = InParams.GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_DamageMultiplier, 1.f);
    Damage *= SpawnDamageMultiplier;

    float DurationMultiplier = InParams.GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_DurationMultiplier, 1.f);
    LifeSpan *= DurationMultiplier;
}

void ADiaSkillActor::ConfigureSkillActor(const FDiaSkillActorConfigureData& Config)
{
	RuntimeParams = Config.RuntimeParams;

	if (Config.Duration >= 0.f)
	{
		const float DurationMultiplier = RuntimeParams.GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_DurationMultiplier, 1.f);
		LifeSpan = Config.Duration * DurationMultiplier;
	}

	if (Config.HitInterval >= 0.f)
	{
		IntervalBetweenHits = FMath::Max(0.01f, Config.HitInterval);
	}

	if (Config.MaxHitCount > 0)
	{
		MaxHitCount = Config.MaxHitCount;
	}
}

void ADiaSkillActor::ArmRemovalTimer(float Seconds)
{
	if (Seconds <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	SetLifeSpan(0.f);
	World->GetTimerManager().ClearTimer(LifeSpanTimerHandle);
	World->GetTimerManager().SetTimer(
		LifeSpanTimerHandle,
		this,
		&ADiaSkillActor::OnSkillObjectRemovalTimer,
		Seconds,
		false);
}

void ADiaSkillActor::ExplodeAdditioanlly(float Radius)
{
    if (Radius <= 0.f)
    {
        return;
    }
    TArray<FOverlapResult> Overlaps;
    FCollisionShape CollisionShape = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    QueryParams.AddIgnoredActor(GetOwner());
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    bool bHasOverlaps = World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_EngineTraceChannel2,
        CollisionShape,
        QueryParams);
    if (bHasOverlaps)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            if (AActor* OverlappedActor = Overlap.GetActor())
            {
                if (IsValidTarget(OverlappedActor))
                {
                    FHitResult HitResult;
                    HitResult.ImpactPoint = GetActorLocation();
                    HitResult.ImpactNormal = FVector::UpVector; // 간단히 위쪽으로 설정
                    ApplyAdditionalHit(OverlappedActor, HitResult, Cast<ADiaBaseCharacter>(GetOwner()));
                }
            }
        }
	}
}

void ADiaSkillActor::OnSkillObjectRemovalTimer()
{
	Destroy();
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
        SkillAbilityEffectComp->SetVariableFloat(FName(TEXT("EffectScale")), 1.0f);
        SkillAbilityEffectComp->Activate(true);
		ARPG_SKILL_VLOG(TEXT("SkillEffect activated. Actor=%s, Effect=%s"), *GetNameSafe(this), *GetNameSafe(SkillEffect));
    }
    else
    {
        bool isSkillEffectValid = IsValid(SkillEffect);
		FString NameString = isSkillEffectValid ? SkillEffect->GetName() : TEXT("None");
		ARPG_SKILL_VLOG(TEXT("SkillEffect or SkillAbilityEffectComp is not valid. Actor=%s, Effect=%s"),
			*GetNameSafe(this), *NameString);
    }

    // 액터의 수명 설정
    SetLifeSpan(LifeSpan);

    // 효과 자산 확인 및 활성화
    if (IsValid(LagacySkillAbilityEffectComp) && LegacySkillEffect)
    {
        if (LegacySkillEffect)
        {
            LagacySkillAbilityEffectComp->SetTemplate(LegacySkillEffect);
            LagacySkillAbilityEffectComp->Activate(true);
        }
    }
}

void ADiaSkillActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LifeSpanTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
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


    if (IsSpawnedByFork())
		ApplyAdditionalHit(OtherActor, HitResult, OwnerActor);
    else
        ApplyGameplayHit(OtherActor, HitResult, OwnerActor);
}

void ADiaSkillActor::ApplyGameplayHit(AActor* OtherActor, const FHitResult& HitResult, ADiaBaseCharacter* OwnerActor)
{
    IAbilitySystemInterface* DiaOtherActor = Cast<IAbilitySystemInterface>(OtherActor);
    if (DiaOtherActor)
    {
		LogHitState(OtherActor, TEXT("ApplyGameplayHit"));

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
                VariantContext.HitActor = OtherActor;
                VariantContext.SkillActorClass = OwningAbilityPtr->GetSkillActorClassForSpawn();
				VariantContext.SkillActors.Add(this);
                VariantContext.AbilityComp = OwnerActor->GetAbilitySystemComponent();

                const FVector HitLocation = HitResult.ImpactPoint.IsNearlyZero()
                    ? OtherActor->GetActorLocation()
                    : FVector(HitResult.ImpactPoint);
                const FVector ForkDirection = GetActorForwardVector().GetSafeNormal();

                FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
                LocationData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
                LocationData->SourceLocation.LiteralTransform = FTransform(ForkDirection.Rotation(), HitLocation);
                LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
                LocationData->TargetLocation.LiteralTransform = FTransform(ForkDirection.Rotation(), HitLocation + ForkDirection * 100.f);
                VariantContext.TargetData.Add(LocationData);
                
                FSkillHitRuntime HitRuntime;
                HitRuntime.PierceCount = PierceCount; // 현재 PierceCount 설정
                OwnerActor->GetSkillManagerComponent()->HitSkillActorUseVariants(VariantContext, OwningAbilityPtr, HitRuntime);
                
                // Runtime의 PierceCount를 SkillActor에 반영
                PierceCount = HitRuntime.PierceCount;
            }
        }

        // Pierce: PierceCount가 0 이하면 발사체 파괴
        if (PierceCount <= 0)
        {
            FName SkillActorTag = FDiaGameplayTags::Get().SkillActor_Ground.GetTagName();
            if (!Tags.Contains(SkillActorTag))
            {
				ARPG_SKILL_VVLOG(TEXT("Destroying projectile after pierce exhausted. %s"), *GetDebugString());
                // 땅에 닿는 스킬 액터는 파괴 안함
                Destroy();
            }
        }
    }
}

void ADiaSkillActor::ApplyAdditionalHit(AActor* OtherActor, const FHitResult& HitResult, ADiaBaseCharacter* OnwerActor)
{
    IAbilitySystemInterface* DiaOtherActor = Cast<IAbilitySystemInterface>(OtherActor);
    if (DiaOtherActor)
    {
		LogHitState(OtherActor, TEXT("ApplyAdditionalHit"));

        // 이미 히트한 적 목록에 추가
        HitActors.Add(OtherActor);

        // 데미지 처리
        ProcessDamage(DiaOtherActor, HitResult);

        // 피격 이펙트 생성
        SpawnHitEffect(HitResult.ImpactPoint, HitResult.ImpactNormal);

        //피격시 효과 타겟에게 적용.
        ProcessTargetEffects(DiaOtherActor);
    }
}

void ADiaSkillActor::OnHitDetect()
{
}

FString ADiaSkillActor::GetDebugString() const
{
	return FString::Printf(
		TEXT("SkillActor=%s Owner=%s Damage=%.2f Multiplier=%.2f Pierce=%d Fork=%d HitCount=%d/%d"),
		*GetNameSafe(this),
		*GetNameSafe(GetOwner()),
		Damage,
		DamageMultiplier,
		PierceCount,
		ForkCount,
		HitCount,
		MaxHitCount);
}

void ADiaSkillActor::LogHitState(AActor* TargetActor, const TCHAR* Reason) const
{
	ARPG_SKILL_VVLOG(TEXT("%s Target=%s %s"), Reason, *GetNameSafe(TargetActor), *GetDebugString());
}

void ADiaSkillActor::OnSkillHit(IAbilitySystemInterface* HitActor, const FHitResult& HitResult)
{
}

void ADiaSkillActor::ProcessDamage(IAbilitySystemInterface* ASCInterface, const FHitResult& HitResult)
{
    if (!(ASCInterface) || !IsValid(Owner))
    {
		ARPG_SKILL_LOG(Warning, TEXT("ProcessDamage skipped: invalid target interface or owner. Actor=%s, TargetInterface=%s, Owner=%s"),
			*GetNameSafe(this),
			ASCInterface ? TEXT("Valid") : TEXT("None"),
			*GetNameSafe(Owner));
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
            if (TargetASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Invincible))
            {
				ARPG_SKILL_LOG(Warning, TEXT("ProcessDamage skipped: target is invincible. Actor=%s, TargetASC=%s"),
					*GetNameSafe(this),
					*GetNameSafe(TargetASC));
				return;
            }

            FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
            EffectContext.AddInstigator(GetOwner(), Cast<APawn>(GetOwner()));

            FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGameplayEffect, 1.0f, EffectContext);
            if (SpecHandle.IsValid())
            {
                // SetByCaller로 기본 대미지 주입(Exec_Damage가 참조 가능)
                SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("GASData.DamageBase")), Damage);

                // 필요 시 치명타 배수 등 추가 세팅 가능
                SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("GASData.DamageMultiplier")), DamageMultiplier);

                SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

				ARPG_SKILL_VLOG(TEXT("Applied damage effect. TargetASC=%s, DamageEffect=%s %s"),
					*GetNameSafe(TargetASC),
					*GetNameSafe(DamageGameplayEffect),
					*GetDebugString());
            }
            else
            {
				ARPG_SKILL_LOG(Warning, TEXT("ProcessDamage skipped: failed to make outgoing spec. Actor=%s, DamageEffect=%s, SourceASC=%s"),
					*GetNameSafe(this),
					*GetNameSafe(DamageGameplayEffect),
					*GetNameSafe(SourceASC.Get()));
            }
        }
        else
        {
			ARPG_SKILL_LOG(Warning, TEXT("ProcessDamage skipped: target ASC is null. Actor=%s"), *GetNameSafe(this));
        }
    }
    else
    {
		ARPG_SKILL_LOG(Warning, TEXT("ProcessDamage skipped: SourceASC or DamageGameplayEffect is invalid. Actor=%s, SourceASC=%s, DamageEffect=%s, Damage=%.2f"),
			*GetNameSafe(this),
			*GetNameSafe(SourceASC.Get()),
			*GetNameSafe(DamageGameplayEffect),
			Damage);
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
            NiagaraComp->SetVariableFloat(FName(TEXT("EffectScale")), 1.0f);
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
        static const FName CharacterTag = FName(TEXT("Character")); 
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
