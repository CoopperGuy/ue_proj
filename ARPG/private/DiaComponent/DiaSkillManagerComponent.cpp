// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaComponent/Skill/Executor/DiaSkillVariantSpawnExecutor.h"
#include "DiaComponent/Skill/Executor/DiaSkillHitVariantExecutor.h"
#include "DiaComponent/Service/DiaSkillLoadService.h"
#include "DiaComponent/Service/DiaSkillActivationService.h"
#include "DiaComponent/Service/DiaSkillVariantExecutorService.h"
#include "DiaComponent/Service/DiaSkillVariantCache.h"

#include "Types/DiaGASSkillData.h"
#include "Skill/DiaSkillActor.h"

#include "GAS/DiaGameplayAbility.h"

#include "System/GASSkillManager.h"
#include "System/JobSkillSetSubSystem.h"
#include "DiaBaseCharacter.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"

#include "Engine/World.h"

#include "GameplayAbilitySpec.h"


UDiaSkillManagerComponent::UDiaSkillManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SkillIDMapping.Reserve(MaxSkillMapping);
}


void UDiaSkillManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Service 인스턴스 생성 (생성자에서 NewObject 사용 시 오류 발생하므로 BeginPlay에서 생성)
	LoadService = NewObject<UDiaSkillLoadService>(this);
	ActivationService = NewObject<UDiaSkillActivationService>(this);
	VariantExecutor = NewObject<UDiaSkillVariantExecutorService>(this);
	VariantCache = NewObject<UDiaSkillVariantCache>(this);
}

void UDiaSkillManagerComponent::MakeSkillVariantsArray(IN const UDiaGameplayAbility* Ability, OUT TArray<UDiaSkillVariant*>& OutVariantsArray)
{
	if (!IsValid(Ability))
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: Ability가 유효하지 않습니다."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: World가 유효하지 않습니다."));
		return;
	}

	const USkillObject* SkillObj = Ability->GetSkillObject();
	if (!IsValid(SkillObj))
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: SkillObject가 유효하지 않습니다."));
		return;
	}

	TArray<int32> VariantIDs = SkillObj->GetSkillVariantIDs();
	constexpr int32 EstimatedNumVariants = 6;
	OutVariantsArray.Reserve(EstimatedNumVariants);
	if (VariantIDs.Num() > 0)
	{
		for (const int32& VariantID : VariantIDs)
		{
			if (UDiaSkillVariant* FoundVariant = SkillVariants.FindRef(VariantID))
			{
				OutVariantsArray.Add(FoundVariant);
			}
		}
	}
}

void UDiaSkillManagerComponent::LoadJobSKillDataFromTable(EJobType JobType)
{
	SkillIDMapping.Reset();
	SkillVariants.Reset();
	LoadService->LoadJobSkillData(JobType, SkillIDMapping, SkillVariants, this);
}

const int32 UDiaSkillManagerComponent::GetMappedSkillID(int32 Index) const
{
	if (SkillIDMapping.IsValidIndex(Index))
	{
		return SkillIDMapping[Index]->GetSkillID();
	}
	return -1; // 유효하지 않은 인덱스일 경우 -1 반환
}

const int32 UDiaSkillManagerComponent::GetIndexOfSkillID(int32 SkillID) const
{
	for (size_t i = 0; i < SkillIDMapping.Num(); i++)
	{
		if(SkillIDMapping[i]->GetSkillID() == SkillID)
		{
			return static_cast<int32>(i);
		}
	}

	return -1;
}

const USkillObject* UDiaSkillManagerComponent::GetSkillObjectBySkillID(int32 SkillID) const
{
	for (const auto& SkillObject : SkillIDMapping)
	{
		if (SkillObject->GetSkillID() == SkillID)
		{
			return SkillObject;
		}
	}
	return nullptr;
}

const TArray<USkillObject*>& UDiaSkillManagerComponent::GetSkillIDMapping() const
{
	return SkillIDMapping;
}

void UDiaSkillManagerComponent::SetSkillIDMapping(const TArray<int32>& NewMapping)
{
	for(const auto& SkillID : NewMapping)
	{
		USkillObject* NewSkillObject = NewObject<USkillObject>(this);
		NewSkillObject->SetSkillID(SkillID);
		SkillIDMapping.Add(NewSkillObject);
	}
}

void UDiaSkillManagerComponent::SetSkillIDIndex(int32 SkillID , int32 Index)
{
	UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SetSkillIDIndex: Index %d, SkillID %d"), Index, SkillID);
	if (SkillIDMapping.IsValidIndex(Index))
	{
		USkillObject* NewSkillObject = NewObject<USkillObject>(this);
		NewSkillObject->SetSkillID(SkillID);
		SkillIDMapping[Index] = NewSkillObject;
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SetSkillIDIndex: SkillIDMapping[%d] set to SkillID %d"), Index, SkillID);
	}
}

FGameplayAbilitySpec* UDiaSkillManagerComponent::GetAbilitySpecBySkillID(int32 SkillID) const
{
	ADiaBaseCharacter* OwnerCharacter = Cast<ADiaBaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return nullptr;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		return nullptr;
	}

	// Search by input ID (we're using skill ID as input ID)
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.InputID == SkillID)
		{
			return &Spec;
		}
	}

	return nullptr;
}

void UDiaSkillManagerComponent::SpawnSkillActorUseVariants(const FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability)
{
	const USkillObject* SkillObj = Ability->GetSkillObject();
	if (!SkillObj)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: SkillObject가 유효하지 않습니다."));
		return;
	}

	FDiaSkillVariantContext& MutableContext = const_cast<FDiaSkillVariantContext&>(context);
	VariantExecutor->ExecuteVariants(SkillObj->GetSkillVariantIDs(), SkillVariants, MutableContext, Ability);
}

void UDiaSkillManagerComponent::HitSkillActorUseVariants(const FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability)
{
	if (!context.SkillActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: SkillActorClass가 유효하지 않습니다."));
		return;
	}

	TArray<UDiaSkillVariant*> VariantsToApply;
	MakeSkillVariantsArray(Ability, VariantsToApply);

	FDiaSkillVariantContext& MutableContext = const_cast<FDiaSkillVariantContext&>(context);
	UDiaSkillHitVariantExecutor* HitExecutor = NewObject<UDiaSkillHitVariantExecutor>(this);
	HitExecutor->ExecuteEffect(VariantsToApply, MutableContext, Ability);
}

void UDiaSkillManagerComponent::GetSkillVariantFromID(const int32 VariantID, OUT UDiaSkillVariant* OutVariants)
{
	if (UDiaSkillVariant** FoundVariant = SkillVariants.Find(VariantID))
	{
		OutVariants = *FoundVariant;
	}
}

void UDiaSkillManagerComponent::GetSkillVariantsFromSkillID(const int32 SkillID, OUT TArray<UDiaSkillVariant*>& OutVariants)
{
	//스킬 id로 uskillobject를 찾고 SkillVariants 반환
	const USkillObject* SkillObj = GetSkillObjectBySkillID(SkillID);
	if (!SkillObj)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::GetSkillVariantFromSkillID: SkillObject가 유효하지 않습니다."));
		return;
	}
	TArray<int32> VariantIDs = SkillObj->GetSkillVariantIDs();
	for (const int32& VariantID : VariantIDs)
	{
		if (UDiaSkillVariant* FoundVariant = SkillVariants.FindRef(VariantID))
		{
			OutVariants.Add(FoundVariant);
		}
	}
}

bool UDiaSkillManagerComponent::TryActivateAbilityBySkillID(int32 SkillID)
{
	const USkillObject* SkillObj = GetSkillObjectBySkillID(SkillID);
	return ActivationService->TryActivateSkill(SkillID, Cast<ADiaBaseCharacter>(GetOwner()), SkillObj);
}


