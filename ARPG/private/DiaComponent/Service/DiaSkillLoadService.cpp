// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Service/DiaSkillLoadService.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "System/GASSkillManager.h"
#include "System/JobSkillSetSubSystem.h"
#include "Engine/World.h"
#include "Logging/ARPGLogChannels.h"

void UDiaSkillLoadService::LoadJobSkillData(
	EJobType JobType,
	TArray<USkillObject*>& OutSkillObjects,
	TMap<int32, UDiaSkillVariant*>& OutSkillVariants,
	UObject* OuterObject)
{
	UWorld* World = OuterObject ? OuterObject->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillLoadService::LoadJobSkillData: World가 유효하지 않습니다."));
		return;
	}

	UGameInstance* Instance = World->GetGameInstance();
	if (!IsValid(Instance))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillLoadService::LoadJobSkillData: GameInstance가 유효하지 않습니다."));
		return;
	}

	UJobSkillSetSubSystem* JobSkillSubSystem = Instance->GetSubsystem<UJobSkillSetSubSystem>();
	if (!IsValid(JobSkillSubSystem))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillLoadService::LoadJobSkillData: UJobSkillSetSubSystem가 유효하지 않습니다."));
		return;
	}

	UGASSkillManager* GASSkillManager = Instance->GetSubsystem<UGASSkillManager>();
	if (!IsValid(GASSkillManager))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillLoadService::LoadJobSkillData: UGASSkillManager가 유효하지 않습니다."));
		return;
	}

	FJobSkillSet JobSkillSet = JobSkillSubSystem->GetJobSkillSet(JobType);

	for (const int32 SkillID : JobSkillSet.SkillIDs)
	{
		USkillObject* NewSkillObject = NewObject<USkillObject>(OuterObject);
		NewSkillObject->SetSkillID(SkillID);

		const FGASSkillData* GASData = GASSkillManager->GetSkillDataPtr(SkillID);
		if (GASData)
		{
			NewSkillObject->SetSkillVariantIDs(GASData->VariantIDs);
		}

		OutSkillObjects.Add(NewSkillObject);

		// 게임에서 사용할 variant 객체들 미리 생성
		if (GASData)
		{
			for (const int32 VariantID : GASData->VariantIDs)
			{
				if (OutSkillVariants.Find(VariantID) == nullptr)
				{
					const FSkillVariantData* Data = GASSkillManager->GetSkllVariantDataPtr(VariantID);
					if (Data)
					{
						const UScriptStruct* DataExtraStruct = Data->VariantExtraData.GetScriptStruct();
						if (const FSkillVariantGroundSpawnData* GroundSpawnData = Data->GetVariantExtraPtr<FSkillVariantGroundSpawnData>())
						{
							ARPG_SKILL_VLOG(TEXT("LoadSkillVariant ground extra data. VariantID=%d, Tag=%s, ExtraDataStruct=%s, SkillActorClass=%s, Radius=%.2f, Duration=%.2f, TickInterval=%.2f, DamageEffect=%s"),
								VariantID,
								*Data->VariantTag.ToString(),
								*GetNameSafe(DataExtraStruct),
								*GetNameSafe(GroundSpawnData->SkillActorClass),
								GroundSpawnData->Radius,
								GroundSpawnData->Duration,
								GroundSpawnData->TickInterval,
								*GetNameSafe(GroundSpawnData->DamageEffectClass));
						}
						else if (Data->VariantExtraData.IsValid())
						{
							ARPG_SKILL_VLOG(TEXT("LoadSkillVariant extra data has non-ground type. VariantID=%d, Tag=%s, ExtraDataStruct=%s"),
								VariantID,
								*Data->VariantTag.ToString(),
								*GetNameSafe(DataExtraStruct));
						}

						FDiaSkillVariantSpec Spec;
						Spec.ModifierValue = Data->ModifierValue;
						Spec.SkillTag = Data->VariantTag;
						Spec.VariantExtraData = Data->VariantExtraData;

						UDiaSkillVariant* NewDiaSkillVariant = NewObject<UDiaSkillVariant>(OuterObject);
						NewDiaSkillVariant->InitializeVariant(Spec);
						const FDiaSkillVariantSpec& InitializedSpec = NewDiaSkillVariant->GetVariantSpec();
						const FSkillVariantGroundSpawnData* InitializedGroundSpawnData = InitializedSpec.GetVariantExtraPtr<FSkillVariantGroundSpawnData>();
						if (InitializedGroundSpawnData)
						{
							ARPG_SKILL_VLOG(TEXT("LoadSkillVariant initialized ground extra data. VariantID=%d, Tag=%s, ExtraDataStruct=%s, SkillActorClass=%s"),
								VariantID,
								*InitializedSpec.SkillTag.ToString(),
								*GetNameSafe(InitializedSpec.VariantExtraData.GetScriptStruct()),
								*GetNameSafe(InitializedGroundSpawnData->SkillActorClass));
						}

						NewDiaSkillVariant->SetSkillID(VariantID);
						NewDiaSkillVariant->SetSkillVariantName(Data->VariantName);
						NewDiaSkillVariant->SetSkillVariantDescription(Data->Description);

						OutSkillVariants.Add(VariantID, NewDiaSkillVariant);
					}
				}
			}
		}
	}
}
