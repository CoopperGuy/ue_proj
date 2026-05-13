// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Executor/DiaSkillHitVariantExecutor.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect_Hit.h"
#include "DiaComponent/Skill/DiaSkillActorSpawnHelper.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"
#include "Skill/DiaSkillActor.h"

void UDiaSkillHitVariantExecutor::InitializeExecutor()
{
	UE_LOG(LogTemp, Log, TEXT("UDiaSkillHitVariantExecutor::InitializeExecutor - Initializing Hit Variant Executor"));
	RegisteredEffects.Add(NewObject<UDiaSkillVariantEffect_Pierce>(this));
	RegisteredEffects.Add(NewObject<UDiaSkillVariantEffect_Explosion>(this));	
}

void UDiaSkillHitVariantExecutor::ExecuteEffect(const TArray<class UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, UDiaGameplayAbility* Ability)
{
	FSkillHitRuntime Runtime;
	ApplyEffects(Variants, Context, Runtime);
}

void UDiaSkillHitVariantExecutor::ExecuteEffect(const TArray<class UDiaSkillVariant*>& Variants, FDiaSkillVariantContext& Context, const UDiaGameplayAbility* Ability, FSkillVariantRuntime& OutRuntime)
{
	FSkillHitRuntime& HitRuntime = static_cast<FSkillHitRuntime&>(OutRuntime);
	// 기존 PierceCount를 Runtime에 설정
	TArray<TObjectPtr<ADiaSkillActor>> SkillActorsSnapshot;

	for (const ADiaSkillActor* SkillActor : Context.SkillActors)
	{
		if (SkillActor)
		{
			HitRuntime.PierceCount = SkillActor ? SkillActor->GetPierceCount() : 0;
			HitRuntime.ForkCount = SkillActor ? SkillActor->GetForkCount() : 0;
			ApplyEffects(Variants, Context, HitRuntime);
			if (!SkillActor->IsSpawnedByFork() && HitRuntime.ForkCount > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("UDiaSkillHitVariantExecutor::ExecuteEffect - Forking Skill with ForkCount: %d"), HitRuntime.ForkCount);
				FSkillSpawnRuntime SpawnRuntime;
				SpawnRuntime.AngleOffset = 30.f; // 포킹 시 발사체 간의 각도 간격 (예시값)
				SpawnRuntime.PierceCount = HitRuntime.PierceCount; // 포킹
				SpawnRuntime.DamageMultiplier = 1.f; // 포킹 시 데미지 배율 (예시값)
				SpawnRuntime.ExtraSpawnCount = HitRuntime.ForkCount; // ForkCount만큼 추가 발사체 생성`

				const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
				AActor* Character = (ActorInfo.AvatarActor.Get());
				APawn* Pawn = Cast<APawn>(Character);
				FSpawnLocationData SpawnLocationData = FDiaSkillActorSpawnHelper::CalculateSpawnLocationData(Context, Character, SpawnRuntime);
				FVector Direction = SpawnLocationData.Direction;

				UWorld* World = Ability ? Ability->GetWorld() : nullptr;
				if (!World)
				{
					UE_LOG(LogTemp, Warning, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - World가 유효하지 않습니다."));
					return;
				}

				for (int32 i = 0; i < SpawnRuntime.ExtraSpawnCount; i++)
				{
					const FTransform SpawnTransform = FDiaSkillActorSpawnHelper::MakeSpawnTransform(SpawnLocationData, Direction);

					UE_LOG(LogTemp, Log, TEXT("UDiaSkillVariantSpawnExecutor::ExecuteEffect - SkillID: %d, Class: %s, SpawnLocation: %s, Direction: %s"),
						Ability ? Ability->GetSkillData().SkillID : -1,
						*GetNameSafe(Context.SkillActorClass),
						*SpawnTransform.GetLocation().ToString(),
						*Direction.ToString());

					TArray<FGameplayEffectSpecHandle> TargetEffectSpecs;
					Ability->MakeEffectSpecContextToTarget(TargetEffectSpecs);
					FDiaSkillActorSpawnHelper::ApplyVariantEffects(Variants, Context, Ability, SpawnRuntime, TargetEffectSpecs);

					FDiaSkillActorSpawnRequest SpawnRequest;
					SpawnRequest.World = World;
					SpawnRequest.SkillActorClass = Context.SkillActorClass;
					SpawnRequest.SpawnTransform = SpawnTransform;
					SpawnRequest.OwnerActor = Character;
					SpawnRequest.Instigator = Pawn;
					SpawnRequest.SkillData = &Ability->GetSkillData();
					SpawnRequest.SourceASC = ActorInfo.AbilitySystemComponent.Get();
					SpawnRequest.DamageEffectClass = Ability->GetDamageEffectClass();
					SpawnRequest.OwningAbility = const_cast<UDiaGameplayAbility*>(Ability);
					SpawnRequest.TargetEffectSpecs = MoveTemp(TargetEffectSpecs);
					SpawnRequest.IgnoredActors.Add(Context.HitActor);
					SpawnRequest.DamageMultiplier = SpawnRuntime.DamageMultiplier;
					SpawnRequest.PierceCount = SpawnRuntime.PierceCount + 1;
					SpawnRequest.bSpawnedByFork = true;

					if (ADiaSkillActor* SpawnedActor = FDiaSkillActorSpawnHelper::SpawnSkillActor(SpawnRequest))
					{
						SkillActorsSnapshot.Add(SpawnedActor);
					}
					// 다음 발사체를 위한 방향 업데이트 (다음 루프에서 사용)
					Direction = SpawnLocationData.NextProjectileRotator.RotateVector(Direction).GetSafeNormal();
				}
			}
		}
	}
	Context.SkillActors.Append(SkillActorsSnapshot);
}
