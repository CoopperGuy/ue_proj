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

#include "DiaBaseCharacter.h"

void UDiaSkillVariantSpawnExecutor::ExecuteEffect(const TArray<class UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability)
{
	if(Context.SkillActorClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - SkillActorClass is null"));
		return;
	}
	constexpr float AdditionalAngle = 10.f;
	constexpr float defaultDist = 10.f;

	FSkillSpawnRuntime Runtime;
	Runtime.ExtraSpawnCount = 4;
	ApplyEffects(Variants, Context, Runtime);	

	const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
	AActor* Character = (ActorInfo.AvatarActor.Get());
	APawn* Pawn = Cast<APawn>(Character);
	
	// 캐릭터 위치를 스폰 기준점으로 사용
	FVector CharacterLocation = Character ? Character->GetActorLocation() : FVector::ZeroVector;
	FVector CharacterForward = Character ? Character->GetActorForwardVector() : FVector::ForwardVector;
	
	FVector TargetLocation = FVector::ZeroVector;
	FVector Direction = FVector::ForwardVector;

	// 타겟 위치와 방향 가져오기
	FTransform SpawnTransform;
	if (const FGameplayAbilityTargetData* LocationData = Context.TargetData.Get(0))
	{
		if (LocationData->HasHitResult())
		{
			TargetLocation = LocationData->GetHitResult()->Location;
			Direction = (TargetLocation - CharacterLocation).GetSafeNormal();
		}
		else if (LocationData->HasEndPoint())
		{
			TargetLocation = LocationData->GetEndPointTransform().GetLocation();
			Direction = (TargetLocation - CharacterLocation).GetSafeNormal();
		}
	}
	
	// 타겟 방향이 없으면 캐릭터의 앞 방향 사용
	if (Direction.IsNearlyZero())
	{
		Direction = CharacterForward;
	}

	FRotator AdditionalRotator = FRotator(0.f, -AdditionalAngle * 0.5f * (Runtime.ExtraSpawnCount - 1), 0.f);
	FRotator DefaultRotator = FRotator(0.f, AdditionalAngle, 0.f);

	// 초기 방향 설정
	Direction = AdditionalRotator.RotateVector(Direction).GetSafeNormal();

	UWorld* World = Ability ? Ability->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - World가 유효하지 않습니다."));
		return;
	}

	for (int32 i = 0; i < Runtime.ExtraSpawnCount; i++)
	{				
		// 각 발사체마다 캐릭터 위치 기준으로 고유한 위치 계산
		FVector SpawnLocation = CharacterLocation + Direction * defaultDist;
		SpawnTransform = FTransform(Direction.Rotation(), SpawnLocation);
		
		AActor* SpawnedActorRaw = World->SpawnActorDeferred<AActor>(
			Context.SkillActorClass,
			SpawnTransform,
			Character,
			Pawn,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

		ADiaSkillActor* SpawnedActor = Cast<ADiaSkillActor>(SpawnedActorRaw);
		if (SpawnedActor)
		{
			const FGASSkillData& SkillData = Ability->GetSkillData();
			UAbilitySystemComponent* SourceASC = ActorInfo.AbilitySystemComponent.Get();
			const TSubclassOf<UGameplayEffect> DamageEffectClass = Ability->GetDamageEffectClass();

			//상대방에게 전달할 이펙트 생성
			TArray<FGameplayEffectSpecHandle> TargetEffectSpecs;
			Ability->MakeEffectSpecContextToTarget(TargetEffectSpecs);
			SpawnedActor->InitTargetEffectHandle(TargetEffectSpecs);
			SpawnedActor->Initialize(SkillData, Character, SourceASC, DamageEffectClass);
			SpawnedActor->Launch(SpawnTransform.GetRotation().GetForwardVector());
			SpawnedActor->SetOwner(Character);
			SpawnedActor->SetOwningAbility(Ability);

			// Pierce Variant 확인 및 PierceCount 초기화
			for (const UDiaSkillVariant* Variant : Variants)
			{
				if (Variant)
				{
					const FDiaSkillVariantSpec Spec = Variant->GetVariantSpec();
					if (Spec.SkillTag == FDiaGameplayTags::Get().GASData_Pierce)
					{
						// ModifierValue는 관통 횟수 (최소 1)
						SpawnedActor->SetPierceCount(FMath::Max(1, static_cast<int32>(Spec.ModifierValue)));
						break;
					}
				}
			}

			SpawnedActor->FinishSpawning(SpawnTransform);
			
		}	
		// 다음 발사체를 위한 방향 업데이트 (다음 루프에서 사용)
		Direction = DefaultRotator.RotateVector(Direction).GetSafeNormal();
	}
}
