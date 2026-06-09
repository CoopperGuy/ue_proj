// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/Executor/DiaSkillHitVariantExecutor.h"
#include "DiaComponent/Skill/Effect/DiaSkillVariantEffect_Hit.h"
#include "DiaComponent/Skill/DiaSkillActorSpawnHelper.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGameplayTags.h"
#include "Logging/ARPGLogChannels.h"
#include "Skill/DiaSkillActor.h"

void UDiaSkillHitVariantExecutor::InitializeExecutor()
{
	ARPG_SKILL_VLOG(TEXT("Initializing Hit Variant Executor"));
	RegisteredEffects.Add(NewObject<UDiaSkillVariantEffect_Pierce>(this));
	RegisteredEffects.Add(NewObject<UDiaSkillVariantEffect_Explosion>(this));	
	RegisteredEffects.Add(NewObject<UDiaSkillVariantEffect_SpawnGround>(this));
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

	ARPG_SKILL_VLOG(TEXT("HitVariant ExecuteEffect entered. SkillID=%d, VariantCount=%d, ContextSkillActors=%d, SkillActorClass=%s, HitActor=%s"),
		Ability ? Ability->GetSkillData().SkillID : -1,
		Variants.Num(),
		Context.SkillActors.Num(),
		*GetNameSafe(Context.SkillActorClass),
		*ARPGLog::DescribeActor(Context.HitActor));

	for (const UDiaSkillVariant* Variant : Variants)
	{
		if (!Variant)
		{
			ARPG_SKILL_LOG(Warning, TEXT("HitVariant variant list contains null"));
			continue;
		}

		const FDiaSkillVariantSpec Spec = Variant->GetVariantSpec();
		ARPG_SKILL_VLOG(TEXT("HitVariant input variant. Tag=%s, ModifierValue=%.2f, Variant=%s"),
			*Spec.SkillTag.ToString(),
			Spec.ModifierValue,
			*GetNameSafe(Variant));
	}

	UWorld* World = Ability ? Ability->GetWorld() : nullptr;
	if (!World)
	{
		ARPG_SKILL_LOG(Warning, TEXT("Cannot fork skill: World is invalid"));
		return;
	}

	for (const ADiaSkillActor* SkillActor : Context.SkillActors)
	{
		if (SkillActor)
		{
			HitRuntime.PierceCount = SkillActor ? SkillActor->GetPierceCount() : 0;
			HitRuntime.ForkCount = SkillActor ? SkillActor->GetForkCount() : 0;
			ApplyEffects(Variants, Context, HitRuntime);

			const float BurningGroundRuntimeValue = HitRuntime.ActorParams.GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_BurningGround);

			if (HitRuntime.ForkCount <= 0)
			{
				ARPG_SKILL_VLOG(TEXT("Fork skipped: ForkCount is zero. SkillActor=%s, BurningGroundRuntime=%.2f"),
					*ARPGLog::DescribeActor(SkillActor),
					static_cast<float>(HitRuntime.ForkCount));
			}

			if (SkillActor->IsSpawnedByFork())
			{
				ARPG_SKILL_VLOG(TEXT("Fork/BurningGround skipped: source SkillActor is already spawned by fork. SkillActor=%s"), *ARPGLog::DescribeActor(SkillActor));
			}
			else if (HitRuntime.ForkCount > 0)
			{

				FSkillSpawnRuntime SpawnRuntime;
				SpawnRuntime.AngleOffset = 30.f; // 포킹 시 발사체 간의 각도 간격 (예시값)
				SpawnRuntime.ActorParams = SkillActor->GetRuntimeParams();
				SpawnRuntime.ActorParams.SetMagnitude(FDiaGameplayTags::Get().GASData_Pierce, HitRuntime.PierceCount + 1); // 포킹
				SpawnRuntime.ExtraSpawnCount = HitRuntime.ForkCount; // ForkCount만큼 추가 발사체 생성

				const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
				AActor* Character = (ActorInfo.AvatarActor.Get());
				APawn* Pawn = Cast<APawn>(Character);
				FSpawnLocationData SpawnLocationData = FDiaSkillActorSpawnHelper::CalculateSpawnLocationData(Context, Character, SpawnRuntime);
				FVector Direction = SpawnLocationData.Direction;

				ARPG_SKILL_VLOG(TEXT("Fork/BurningGround source accepted. SourceSkillActor=%s, Owner=%s, HitActor=%s, ForkCount=%d, PierceCount=%d, BurningGroundRuntime=%.2f"),
					*ARPGLog::DescribeActor(SkillActor),
					*ARPGLog::DescribeActor(Character),
					*ARPGLog::DescribeActor(Context.HitActor),
					HitRuntime.ForkCount,
					HitRuntime.PierceCount,
					BurningGroundRuntimeValue);

				for (int32 i = 0; i < SpawnRuntime.ExtraSpawnCount; i++)
				{
					const FTransform SpawnTransform = FDiaSkillActorSpawnHelper::MakeSpawnTransform(SpawnLocationData, Direction);

					ARPG_SKILL_VLOG(TEXT("Fork spawn request. Index=%d/%d, SkillID=%d, Class=%s, SpawnLocation=%s, Direction=%s"),
						i + 1,
						SpawnRuntime.ExtraSpawnCount,
						Ability ? Ability->GetSkillData().SkillID : -1,
						*GetNameSafe(Context.SkillActorClass),
						*SpawnTransform.GetLocation().ToString(),
						*Direction.ToString());

					TArray<FGameplayEffectSpecHandle> TargetEffectSpecs;
					Ability->MakeEffectSpecContextToTarget(TargetEffectSpecs);
					FDiaSkillActorSpawnHelper::ApplyVariantEffects(Variants, Context, Ability, SpawnRuntime, TargetEffectSpecs);

					ARPG_SKILL_VLOG(TEXT("After ApplyVariantEffects. TargetEffectSpecs=%d, BurningGround=%.2f, Pierce=%d"),
						TargetEffectSpecs.Num(),
						SpawnRuntime.ActorParams.GetMagnitude(FDiaGameplayTags::Get().GASData_Variant_BurningGround),
						SpawnRuntime.ActorParams.GetInt(FDiaGameplayTags::Get().GASData_Pierce, 0));

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
					SpawnRequest.RuntimeParams = SpawnRuntime.ActorParams;
					SpawnRequest.bSpawnedByFork = true;

					if (ADiaSkillActor* SpawnedActor = FDiaSkillActorSpawnHelper::SpawnSkillActor(SpawnRequest))
					{
						ARPG_SKILL_VLOG(TEXT("Fork spawn success. SpawnedActor=%s, Owner=%s, IgnoredActor=%s, Pierce=%d, Fork=%d"),
							*ARPGLog::DescribeActor(SpawnedActor),
							*ARPGLog::DescribeActor(Character),
							*ARPGLog::DescribeActor(Context.HitActor),
							SpawnRuntime.ActorParams.GetInt(FDiaGameplayTags::Get().GASData_Pierce, 0),
							HitRuntime.ForkCount);

						SkillActorsSnapshot.Add(SpawnedActor);
					}
					else
					{
						ARPG_SKILL_LOG(Warning, TEXT("Fork spawn failed. SkillID=%d, Class=%s, SpawnLocation=%s"),
							Ability ? Ability->GetSkillData().SkillID : -1,
							*GetNameSafe(Context.SkillActorClass),
							*SpawnTransform.GetLocation().ToString());
					}
					// 다음 발사체를 위한 방향 업데이트 (다음 루프에서 사용)
					Direction = SpawnLocationData.NextProjectileRotator.RotateVector(Direction).GetSafeNormal();
				}
			}
			
			if (BurningGroundRuntimeValue > 0.f)
			{
				FSkillSpawnRuntime SpawnRuntime;
				SpawnRuntime.AngleOffset = 30.f; // 포킹 시 발사체 간의 각도 간격 (예시값)
				SpawnRuntime.ActorParams = SkillActor->GetRuntimeParams();
				SpawnRuntime.ActorParams.SetMagnitude(FDiaGameplayTags::Get().GASData_Variant_BurningGround, BurningGroundRuntimeValue); // 포킹

				const FGameplayAbilityActorInfo& ActorInfo = Ability->GetActorInfo();
				AActor* Character = (ActorInfo.AvatarActor.Get());
				APawn* Pawn = Cast<APawn>(Character);
				FSpawnLocationData SpawnLocationData = FDiaSkillActorSpawnHelper::CalculateSpawnLocationData(Context, Character, SpawnRuntime);
				FVector Direction = SpawnLocationData.Direction;

				ARPG_SKILL_VLOG(TEXT("BurningGround branch entered. Value=%.2f, CurrentDirection=%s, HitActor=%s"),
					BurningGroundRuntimeValue,
					*Direction.ToString(),
					*ARPGLog::DescribeActor(Context.HitActor));

				TArray<UDiaSkillVariant*> OutBunringGroundVariants;
				const FTransform SpawnTransform = FDiaSkillActorSpawnHelper::MakeSpawnTransform(SpawnLocationData, Direction);
				FDiaSkillVariantUtils::FindAllByTag(Variants, FDiaGameplayTags::Get().GASData_Variant_BurningGround, OutBunringGroundVariants);

				ARPG_SKILL_VLOG(TEXT("BurningGround variants found. Count=%d, SpawnLocation=%s, SpawnRotation=%s"),
					OutBunringGroundVariants.Num(),
					*SpawnTransform.GetLocation().ToString(),
					*SpawnTransform.GetRotation().Rotator().ToString());

				if (OutBunringGroundVariants.Num() > 0)
				{
					const FDiaSkillVariantSpec& BurningGroundSpec = OutBunringGroundVariants[0]->GetVariantSpec();
					const UScriptStruct* ExtraDataScriptStruct = BurningGroundSpec.VariantExtraData.GetScriptStruct();
					ARPG_SKILL_VLOG(TEXT("BurningGround variant spec. Variant=%s, Tag=%s, ModifierValue=%.2f, ExtraDataStruct=%s, ExtraDataValid=%s"),
						*GetNameSafe(OutBunringGroundVariants[0]),
						*BurningGroundSpec.SkillTag.ToString(),
						BurningGroundSpec.ModifierValue,
						*GetNameSafe(ExtraDataScriptStruct),
						BurningGroundSpec.VariantExtraData.IsValid() ? TEXT("true") : TEXT("false"));

					const FSkillVariantGroundSpawnData* BunringVariant = BurningGroundSpec.GetVariantExtraPtr<FSkillVariantGroundSpawnData>();
					if (!BunringVariant)
					{
						ARPG_SKILL_LOG(Warning, TEXT("BurningGround variant extra data is null or wrong type. Variant=%s, ExtraDataStruct=%s"),
							*GetNameSafe(OutBunringGroundVariants[0]),
							*GetNameSafe(ExtraDataScriptStruct));
					}
					else
					{
						const TSubclassOf<UGameplayEffect> BurningGroundDamageEffectClass = BunringVariant->DamageEffectClass
							? BunringVariant->DamageEffectClass
							: Ability->GetDamageEffectClass();

						if (!BunringVariant->DamageEffectClass)
						{
							ARPG_SKILL_LOG(Warning, TEXT("BurningGround DamageEffectClass is null. Falling back to ability damage effect. Variant=%s, Fallback=%s"),
								*GetNameSafe(OutBunringGroundVariants[0]),
								*GetNameSafe(BurningGroundDamageEffectClass));
						}

						ARPG_SKILL_VLOG(TEXT("BurningGround spawn request. Class=%s, Radius=%.2f, Duration=%.2f, TickInterval=%.2f, DamageEffect=%s, TargetEffects=%d"),
							*GetNameSafe(BunringVariant->SkillActorClass),
							BunringVariant->Radius,
							BunringVariant->Duration,
							BunringVariant->TickInterval,
							*GetNameSafe(BurningGroundDamageEffectClass),
							BunringVariant->TargetEffects.Num());

						if (!BunringVariant->SkillActorClass)
						{
							ARPG_SKILL_LOG(Warning, TEXT("BurningGround variant SkillActorClass is null. Variant=%s, ExtraDataStruct=%s, Radius=%.2f, Duration=%.2f, TickInterval=%.2f"),
								*GetNameSafe(OutBunringGroundVariants[0]),
								*GetNameSafe(ExtraDataScriptStruct),
								BunringVariant->Radius,
								BunringVariant->Duration,
								BunringVariant->TickInterval);
						}

						FDiaSkillActorSpawnRequest SpawnRequest;
						SpawnRequest.World = World;
						SpawnRequest.SkillActorClass = BunringVariant->SkillActorClass;
						SpawnRequest.SpawnTransform = SpawnTransform;
						SpawnRequest.OwnerActor = Character;
						SpawnRequest.Instigator = Pawn;
						SpawnRequest.SkillData = &Ability->GetSkillData();
						SpawnRequest.SourceASC = ActorInfo.AbilitySystemComponent.Get();
						SpawnRequest.DamageEffectClass = BurningGroundDamageEffectClass;
						SpawnRequest.OwningAbility = const_cast<UDiaGameplayAbility*>(Ability);
						//SpawnRequest.TargetEffectSpecs = MoveTemp(TargetEffectSpecs);
						SpawnRequest.IgnoredActors.Add(Context.HitActor);
						SpawnRequest.RuntimeParams = SpawnRuntime.ActorParams;
						SpawnRequest.bSpawnedByFork = true;

						if (ADiaSkillActor* SpawnedActor = FDiaSkillActorSpawnHelper::SpawnSkillActor(SpawnRequest))
						{
							FDiaSkillActorConfigureData Config;
							Config.AreaRadius = BunringVariant->Radius;
							Config.Duration = BunringVariant->Duration;
							Config.HitInterval = BunringVariant->TickInterval;
							Config.RuntimeParams = SpawnRuntime.ActorParams;

							SpawnedActor->ConfigureSkillActor(Config);
							SkillActorsSnapshot.Add(SpawnedActor);

							ARPG_SKILL_VLOG(TEXT("BurningGround spawn success. SpawnedActor=%s, Owner=%s, IgnoredActor=%s, AreaRadius=%.2f, Duration=%.2f, HitInterval=%.2f"),
								*ARPGLog::DescribeActor(SpawnedActor),
								*ARPGLog::DescribeActor(Character),
								*ARPGLog::DescribeActor(Context.HitActor),
								Config.AreaRadius,
								Config.Duration,
								Config.HitInterval);
						}
						else
						{
							ARPG_SKILL_LOG(Warning, TEXT("BurningGround spawn failed. Class=%s, SpawnLocation=%s"),
								*GetNameSafe(BunringVariant->SkillActorClass),
								*SpawnTransform.GetLocation().ToString());
						}
					}
				}
				else
				{
					ARPG_SKILL_LOG(Warning, TEXT("BurningGround value is set but no BurningGround variant was found. Value=%.2f"), BurningGroundRuntimeValue);
				}
			}
		}
		else
		{
			ARPG_SKILL_LOG(Warning, TEXT("HitVariant Context.SkillActors contains null"));
		}
	}
	Context.SkillActors.Append(SkillActorsSnapshot);
}
