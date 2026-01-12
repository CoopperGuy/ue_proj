// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Executor/DiaSkillVariantSpawnExecutor.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaComponent/Skill/SkillObject.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/World.h"
#include "GAS/DiaGameplayTags.h"

#include "Types/DiaGASSkillData.h"
#include "Skill/DiaSkillActor.h"
#include "GAS/DiaGameplayAbility.h"

void UDiaSkillVariantSpawnExecutor::ExecuteEffect(const TArray<class UDiaSkillVariant*>& Variants, const FDiaSkillVariantContext& Context, const UDiaGameplayAbility* Ability)
{
	constexpr float AdditionalAngle = 10.f;
	int32 SkillSpawnCount = 1;

	for(const UDiaSkillVariant* Variant : Variants)
	{
		if(Variant)
		{
			const FDiaSkillVariantSpec Spec = Variant->GetVariantSpec();
			UE_LOG(LogTemp, Warning, TEXT("DiaSkillVariantSpawnExecutor::ExecuteEffect: Variant Skill Tag : %s"), *Spec.SkillTag.ToString());
			if(Spec.SkillTag == FDiaGameplayTags::Get().GASData_MultipleShot)
			{
				UE_LOG(LogTemp, Warning, TEXT("DiaSkillVariantSpawnExecutor::ExecuteEffect: MultipleShot Variant 적용, 스폰 개수 증가"));
				SkillSpawnCount++;
			}
		}
	}

	FVector Origin = FVector::ZeroVector;
	FVector Direction = FVector::ForwardVector;

	//SpawnTask에서 가져온 코드
	FTransform SpawnTransform;
	if (const FGameplayAbilityTargetData* LocationData = Context.TargetData.Get(0))		//Hardcode to use data 0. It's OK if data isn't useful/valid.
	{
		//Set location. Rotation is unaffected.
		if (LocationData->HasHitResult())
		{
			SpawnTransform.SetLocation(LocationData->GetHitResult()->Location);
			Origin = LocationData->GetHitResult()->Location;
			Direction = SpawnTransform.GetRotation().GetForwardVector();
		}
		else if (LocationData->HasEndPoint())
		{
			SpawnTransform = LocationData->GetEndPointTransform();
			Origin = LocationData->GetEndPointTransform().GetLocation();
			Direction = SpawnTransform.GetRotation().GetForwardVector();
		}
	}

	FRotator AdditionalRotator = FRotator(0.f, AdditionalAngle * 0.5f * (SkillSpawnCount - 1), 0.f);
	FRotator DefaultRotator = FRotator(0.f, AdditionalAngle, 0.f);

	Direction = AdditionalRotator.RotateVector(Direction);
	SpawnTransform = FTransform(Direction.Rotation(), Origin);

	for (int32 i = 0; i < SkillSpawnCount; i++)
	{
		// Owner와 Instigator 가져오기
		const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
		AActor* OwnerActor = ActorInfo.OwnerActor.Get();
		AActor* InstigatorActor = ActorInfo.AvatarActor.Get();
		
		if (!Context.SkillActorClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("DiaSkillVariantSpawnExecutor::ExecuteEffect: SkillActorClass가 nullptr입니다."));
			continue;
		}
		
		AActor* SpawnedActorRaw = GetWorld()->SpawnActorDeferred<AActor>(
			Context.SkillActorClass,
			SpawnTransform,
			OwnerActor,
			InstigatorActor ? Cast<APawn>(InstigatorActor) : nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		if (!SpawnedActorRaw)
		{
			UE_LOG(LogTemp, Warning, TEXT("DiaSkillVariantSpawnExecutor::ExecuteEffect: SpawnActorDeferred 실패"));
			continue;
		}

		ADiaSkillActor* SpawnedActor = Cast<ADiaSkillActor>(SpawnedActorRaw);
		if (SpawnedActor)
		{
			const FGASSkillData& SkillData = Ability->GetSkillData();
			UAbilitySystemComponent* SourceASC = ActorInfo.AbilitySystemComponent.Get();

			//상대방에게 전달할 이펙트 생성
			TArray<FGameplayEffectSpecHandle> TargetEffectSpecs;
			Ability->MakeEffectSpecContextToTarget(TargetEffectSpecs);
			SpawnedActor->InitTargetEffectHandle(TargetEffectSpecs);
			SpawnedActor->Initialize(SkillData, OwnerActor, SourceASC, Context.SkillActorClass);
			SpawnedActor->Launch(SpawnTransform.GetRotation().GetForwardVector());

			SpawnedActor->FinishSpawning(SpawnTransform);
			
			SpawnedActor->SetOwner(OwnerActor);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DiaSkillVariantSpawnExecutor::ExecuteEffect: Cast<ADiaSkillActor> 실패 - %s"), *SpawnedActorRaw->GetClass()->GetName());
		}

		Direction = DefaultRotator.RotateVector(Direction);
		SpawnTransform = FTransform(Direction.Rotation(), Origin);
	}

}
