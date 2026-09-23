// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/DiaGroundObj.h"

#include "GAS/DiaGameplayTags.h"
#include "GAS/DiaAttributeSet.h"
#include "AbilitySystemComponent.h"

#include "Engine/World.h"
#include "AbilitySystemInterface.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "DiaBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/OverlapResult.h" 
#include "Logging/ARPGLogChannels.h"

ADiaGroundObj::ADiaGroundObj()
{
	CollisionComp->SetSphereRadius(Radius);

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FName SkillActorTag = FDiaGameplayTags::Get().SkillActor_Ground.GetTagName();
	Tags.Add(SkillActorTag);
}

void ADiaGroundObj::BeginPlay()
{
	Super::BeginPlay();
	// Ordinary ground steps do not go through ConfigureSkillActor (hit variants do).
	// Initialize has already supplied the table's hit count and interval.
	if (IntervalBetweenHits > 0.f)
	{
		StartHitDetectTimer();
	}
}

void ADiaGroundObj::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopHitDetectTimer();
	Super::EndPlay(EndPlayReason);
}

void ADiaGroundObj::Initialize(const FGASSkillData& SkillData, AActor* InOwner, UAbilitySystemComponent* InSourceASC, TSubclassOf<UGameplayEffect> InDamageEffect)
{
	Super::Initialize(SkillData, InOwner, InSourceASC, InDamageEffect);
	if (const FGASGroundData* GroundData = SkillData.GetExtraPtr<FGASGroundData>())
	{
		Radius = GroundData->Radius;
		CollisionComp->SetSphereRadius(Radius);
	}
}

void ADiaGroundObj::ApplySpawnRuntimeParams(const FDiaSkillRuntimeParams& InParams)
{
	Super::ApplySpawnRuntimeParams(InParams);
	const float DurationScale = InParams.GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_DurationMultiplier, 1.f);
	MaxHitCount = FMath::Max(1, FMath::CeilToInt(MaxHitCount * DurationScale));
	float SpawnRadiusMultiplier = InParams.GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_AreaMultiplier, 1.f);
	Radius *= SpawnRadiusMultiplier;
	CollisionComp->SetSphereRadius(Radius);
	if (SkilleMesh)
	{
		SkilleMesh->SetRelativeScale3D(SkilleMesh->GetRelativeScale3D() * SpawnRadiusMultiplier);
	}
	if (SkillAbilityEffectComp)
	{
		SkillAbilityEffectComp->SetRelativeScale3D(SkillAbilityEffectComp->GetRelativeScale3D() * SpawnRadiusMultiplier);
	}
}

void ADiaGroundObj::ConfigureSkillActor(const FDiaSkillActorConfigureData& Config)
{
	Super::ConfigureSkillActor(Config);
	if (Config.AreaRadius >= 0.f)
	{
		const float AreaMultiplier = Config.RuntimeParams.GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_AreaMultiplier, 1.f);

		Radius = FMath::Max(0.f, Config.AreaRadius * AreaMultiplier);
		CollisionComp->SetSphereRadius(Radius);
	}

	if (Config.MaxHitCount <= 0 && Config.Duration > 0.f && IntervalBetweenHits > 0.f)
	{
		MaxHitCount = FMath::Max(1, FMath::CeilToInt(LifeSpan / IntervalBetweenHits));
	}

	StartHitDetectTimer();
}

void ADiaGroundObj::StartHitDetectTimer()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		ARPG_SKILL_LOG(Warning, TEXT("Ground hit timer not started: World is invalid. Actor=%s"), *GetNameSafe(this));
		return;
	}

	if (IntervalBetweenHits <= 0.f)
	{
		ARPG_SKILL_LOG(Warning, TEXT("Ground hit timer not started: invalid interval. Actor=%s, Interval=%.2f, Radius=%.2f, Damage=%.2f"),
			*GetNameSafe(this),
			IntervalBetweenHits,
			GetTotalRadius(),
			Damage);
		return;
	}

	World->GetTimerManager().ClearTimer(HitDetectTimerHandle);
	World->GetTimerManager().SetTimer(
		HitDetectTimerHandle,
		this,
		&ADiaGroundObj::OnHitDetect,
		IntervalBetweenHits,
		true,
		0.f);

	ARPG_SKILL_VLOG(TEXT("Ground hit timer started. Actor=%s, Interval=%.2f, Radius=%.2f, MaxHitCount=%d, Damage=%.2f, DamageEffect=%s, SourceASC=%s"),
		*GetNameSafe(this),
		IntervalBetweenHits,
		GetTotalRadius(),
		MaxHitCount,
		Damage,
		*GetNameSafe(DamageGameplayEffect),
		*GetNameSafe(SourceASC.Get()));
}

void ADiaGroundObj::StopHitDetectTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HitDetectTimerHandle);
	}
}

void ADiaGroundObj::OnHitDetect()
{
	++HitCount;
	if(HitCount > MaxHitCount)
	{
		//그냥 히트 판정을 끄자. 그라운드는 그래야한다
		ARPG_SKILL_VLOG(TEXT("MaxHitCount reached. HitCount=%d, MaxHitCount=%d"), HitCount, MaxHitCount);
		StopHitDetectTimer();
		return;
	}

	UWorld* World = GetWorld();
    if (!IsValid(World)) 
		return;

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(Owner);
	bool bHasOverlap = World->OverlapMultiByChannel(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ECollisionChannel::ECC_EngineTraceChannel2,
		FCollisionShape::MakeSphere(GetTotalRadius()),
		QueryParams
	);

	ARPG_SKILL_VLOG(TEXT("Ground hit detect. Actor=%s, HitCount=%d/%d, Radius=%.2f, Overlap=%s, OverlapCount=%d, SourceASC=%s, DamageEffect=%s"),
		*GetNameSafe(this),
		HitCount,
		MaxHitCount,
		GetTotalRadius(),
		bHasOverlap ? TEXT("true") : TEXT("false"),
		OverlapResults.Num(),
		*GetNameSafe(SourceASC.Get()),
		*GetNameSafe(DamageGameplayEffect));

	if (bHasOverlap)
	{
		TSet<AActor*> HitThisTick;
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* OverlappedActor = Result.GetActor();
			if (IsValidTarget(OverlappedActor) && !HitThisTick.Contains(OverlappedActor))
			{
				IAbilitySystemInterface* AbilitySystemActor = Cast<IAbilitySystemInterface>(OverlappedActor);
				if (AbilitySystemActor)
				{
					HitThisTick.Add(OverlappedActor);
					const float Pull = RuntimeParams.GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_PullToCenter);
					UAbilitySystemComponent* TargetASC = AbilitySystemActor->GetAbilitySystemComponent();
					if (Pull > 0.f && HasAuthority() && IsValid(TargetASC) &&
						TargetASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute()) > 0.f &&
						!TargetASC->HasMatchingGameplayTag(FDiaGameplayTags::Get().State_Invincible))
					{
						if (ACharacter* TargetCharacter = Cast<ACharacter>(OverlappedActor))
						{
							FVector TowardCenter = GetActorLocation() - TargetCharacter->GetActorLocation();
							TowardCenter.Z = 0.f;
							if (TowardCenter.SizeSquared() > FMath::Square(50.f) && TargetCharacter->GetCharacterMovement())
							{
								TargetCharacter->GetCharacterMovement()->AddImpulse(TowardCenter.GetSafeNormal() * Pull, true);
							}
						}
					}
					FHitResult HitResult;
					HitResult.ImpactPoint = OverlappedActor->GetActorLocation();
					HitResult.ImpactNormal = (OverlappedActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

					ApplyAdditionalHit(OverlappedActor, HitResult, Cast<ADiaBaseCharacter>(Owner));
				}
			}
		}
	}
}
