// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/DiaGroundObj.h"
#include "Engine/World.h"
#include "AbilitySystemInterface.h"
#include "TimerManager.h"
#include "Components/SphereComponent.h"
#include "DiaBaseCharacter.h"
#include "Engine/OverlapResult.h" 

ADiaGroundObj::ADiaGroundObj()
{
	CollisionComp->SetSphereRadius(Radius);

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
}

void ADiaGroundObj::BeginPlay()
{
	Super::BeginPlay();


}

void ADiaGroundObj::Initialize(const FGASSkillData& SkillData, AActor* InOwner, UAbilitySystemComponent* InSourceASC, TSubclassOf<ADiaSkillActor> InDamageEffect)
{
	Super::Initialize(SkillData, InOwner, InSourceASC, InDamageEffect);
	
	GetWorld()->GetTimerManager().SetTimer(LifeSpanTimerHandle,
		this,
		&ThisClass::OnHitDetect,
		IntervalBetweenHits,
		true);

}

void ADiaGroundObj::OnHitDetect()
{
	++HitCount;
	if(HitCount > MaxHitCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(LifeSpanTimerHandle);
		Destroy();
		UE_LOG(LogTemp, Warning, TEXT("DiaGroundObj::OnHitDetect - MaxHitCount reached. Destroying ground object. HitCount :%d, MaxHitCount : %d"), HitCount, MaxHitCount);
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
		FCollisionShape::MakeSphere(Radius),
		QueryParams
	);

	//디버그용 라인 그리기
	DrawDebugSphere(
		World,
		GetActorLocation(),
		Radius,
		12,
		FColor::Red,
		false,
		0.1f
	);

	if (bHasOverlap)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* OverlappedActor = Result.GetActor();
			if (OverlappedActor && OverlappedActor != this)
			{
				IAbilitySystemInterface* AbilitySystemActor = Cast<IAbilitySystemInterface>(OverlappedActor);
				if (AbilitySystemActor)
				{
					FHitResult HitResult;
					HitResult.ImpactPoint = OverlappedActor->GetActorLocation();
					HitResult.ImpactNormal = (OverlappedActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

					ApplyGameplayHit(OverlappedActor, HitResult, Cast<ADiaBaseCharacter>(Owner));
				}
			}
		}
	}
}