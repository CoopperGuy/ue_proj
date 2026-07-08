// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
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
#include "Logging/ARPGLogChannels.h"

namespace
{
void CacheSkillVariantsForSkillData(UGASSkillManager* GASSkillManager, const FGASSkillData& GASData, TMap<int32, UDiaSkillVariant*>& OutSkillVariants, UObject* OuterObject)
{
	if (!IsValid(GASSkillManager) || !IsValid(OuterObject))
	{
		return;
	}

	for (const int32 VariantID : GASData.VariantIDs)
	{
		if (OutSkillVariants.Contains(VariantID))
		{
			continue;
		}

		const FSkillVariantData* Data = GASSkillManager->GetSkllVariantDataPtr(VariantID);
		if (!Data)
		{
			continue;
		}

		FDiaSkillVariantSpec Spec;
		Spec.ModifierValue = Data->ModifierValue;
		Spec.SkillTag = Data->VariantTag;
		Spec.VariantExtraData = Data->VariantExtraData;

		UDiaSkillVariant* NewDiaSkillVariant = NewObject<UDiaSkillVariant>(OuterObject);
		NewDiaSkillVariant->InitializeVariant(Spec);
		NewDiaSkillVariant->SetSkillID(VariantID);
		NewDiaSkillVariant->SetSkillVariantName(Data->VariantName);
		NewDiaSkillVariant->SetSkillVariantDescription(Data->Description);

		OutSkillVariants.Add(VariantID, NewDiaSkillVariant);
	}
}
}


UDiaSkillManagerComponent::UDiaSkillManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SkillIDMapping.Reserve(MaxOwnedSkillCount);
	QuickSlotSkillIDs.Init(-1, MaxQuickSlotCount);
}


void UDiaSkillManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Service 인스턴스 생성 (생성자에서 NewObject 사용 시 오류 발생하므로 BeginPlay에서 생성)
	if (!IsValid(LoadService))
	{
		LoadService = NewObject<UDiaSkillLoadService>(this);
	}
	if (!IsValid(ActivationService))
	{
		ActivationService = NewObject<UDiaSkillActivationService>(this);
	}
	if (!IsValid(VariantExecutor))
	{
		VariantExecutor = NewObject<UDiaSkillVariantExecutorService>(this);
		VariantExecutor->InitializeExecutorService();
	}
	if (!IsValid(VariantCache))
	{
		VariantCache = NewObject<UDiaSkillVariantCache>(this);
	}
}

void UDiaSkillManagerComponent::MakeSkillVariantsArray(IN const UDiaGameplayAbility* Ability, OUT TArray<UDiaSkillVariant*>& OutVariantsArray)
{
	if (!IsValid(Ability))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: Ability가 유효하지 않습니다."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: World가 유효하지 않습니다."));
		return;
	}

	const USkillObject* SkillObj = Ability->GetSkillObject();
	if (!IsValid(SkillObj))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: SkillObject가 유효하지 않습니다."));
		return;
	}

	TSet<int32> VariantIDs = SkillObj->GetVariantApplyIDs();
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
	QuickSlotSkillIDs.Init(-1, MaxQuickSlotCount);
	if (!IsValid(LoadService))
	{
		LoadService = NewObject<UDiaSkillLoadService>(this);
	}
	if (!IsValid(LoadService))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::LoadJobSKillDataFromTable: LoadService is invalid"));
		return;
	}

	LoadService->LoadJobSkillData(JobType, SkillIDMapping, SkillVariants, this, MaxOwnedSkillCount);

	for (int32 Index = 0; Index < SkillIDMapping.Num() && QuickSlotSkillIDs.IsValidIndex(Index); ++Index)
	{
		if (IsValid(SkillIDMapping[Index]))
		{
			QuickSlotSkillIDs[Index] = SkillIDMapping[Index]->GetSkillID();
		}
	}
}

const int32 UDiaSkillManagerComponent::GetMappedSkillID(int32 Index) const
{
	if (QuickSlotSkillIDs.IsValidIndex(Index))
	{
		return QuickSlotSkillIDs[Index];
	}
	return -1; // 유효하지 않은 인덱스일 경우 -1 반환
}

const int32 UDiaSkillManagerComponent::GetIndexOfSkillID(int32 SkillID) const
{
	for (int32 Index = 0; Index < QuickSlotSkillIDs.Num(); ++Index)
	{
		if (QuickSlotSkillIDs[Index] == SkillID)
		{
			return Index;
		}
	}

	return -1;
}

const USkillObject* UDiaSkillManagerComponent::GetSkillObjectBySkillID(int32 SkillID) const
{
	for (const auto& SkillObject : SkillIDMapping)
	{
		if (IsValid(SkillObject) && SkillObject->GetSkillID() == SkillID)
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

const TArray<int32>& UDiaSkillManagerComponent::GetQuickSlotSkillIDs() const
{
	return QuickSlotSkillIDs;
}

int32 UDiaSkillManagerComponent::FindFirstEmptyQuickSlotIndex() const
{
	for (int32 Index = 0; Index < QuickSlotSkillIDs.Num(); ++Index)
	{
		if (QuickSlotSkillIDs[Index] <= 0)
		{
			return Index;
		}
	}

	return INDEX_NONE;
}

bool UDiaSkillManagerComponent::HasAvailableSkillSlot() const
{
	return SkillIDMapping.Num() < MaxOwnedSkillCount;
}

bool UDiaSkillManagerComponent::TryRegisterSkillByID(int32 SkillID)
{
	if (SkillID <= 0 || GetSkillObjectBySkillID(SkillID) != nullptr || !HasAvailableSkillSlot())
	{
		return false;
	}

	UGASSkillManager* GASSkillManager = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UGASSkillManager>()
		: nullptr;
	if (!IsValid(GASSkillManager))
	{
		return false;
	}

	const FGASSkillData* GASData = GASSkillManager->GetSkillDataPtr(SkillID);
	if (!GASData)
	{
		return false;
	}

	USkillObject* NewSkillObject = NewObject<USkillObject>(this);
	NewSkillObject->SetSkillID(SkillID);
	NewSkillObject->SetSkillVariantIDs(GASData->VariantIDs);
	SkillIDMapping.Add(NewSkillObject);

	CacheSkillVariantsForSkillData(GASSkillManager, *GASData, SkillVariants, this);
	return true;
}

bool UDiaSkillManagerComponent::RemoveSkillByID(int32 SkillID)
{
	const int32 RemovedCount = SkillIDMapping.RemoveAll([SkillID](const USkillObject* SkillObject)
	{
		return IsValid(SkillObject) && SkillObject->GetSkillID() == SkillID;
	});

	if (RemovedCount > 0)
	{
		for (int32& QuickSlotSkillID : QuickSlotSkillIDs)
		{
			if (QuickSlotSkillID == SkillID)
			{
				QuickSlotSkillID = -1;
			}
		}
	}

	return RemovedCount > 0;
}

void UDiaSkillManagerComponent::SetSkillIDMapping(const TArray<int32>& NewMapping)
{
	SkillIDMapping.Reset();
	SkillVariants.Reset();
	QuickSlotSkillIDs.Init(-1, MaxQuickSlotCount);

	int32 SlotIndex = 0;
	for (const int32 SkillID : NewMapping)
	{
		if (TryRegisterSkillByID(SkillID) && QuickSlotSkillIDs.IsValidIndex(SlotIndex))
		{
			QuickSlotSkillIDs[SlotIndex] = SkillID;
			++SlotIndex;
		}
	}
}

void UDiaSkillManagerComponent::SetSkillIDIndex(int32 SkillID , int32 Index)
{
	UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::SetSkillIDIndex: Index %d, SkillID %d"), Index, SkillID);
	if (!QuickSlotSkillIDs.IsValidIndex(Index))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::SetSkillIDIndex: Invalid quick slot index %d"), Index);
		return;
	}

	if (SkillID <= 0)
	{
		QuickSlotSkillIDs[Index] = -1;
		return;
	}

	if (!GetSkillObjectBySkillID(SkillID))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::SetSkillIDIndex: SkillID %d is not registered"), SkillID);
		return;
	}

	QuickSlotSkillIDs[Index] = SkillID;
	UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::SetSkillIDIndex: QuickSlotSkillIDs[%d] set to SkillID %d"), Index, SkillID);
}

void UDiaSkillManagerComponent::NotifySkillRegistered(int32 SkillID, int32 SlotIndex)
{
	OnSkillRegistered.Broadcast(SkillID, SlotIndex);
}

bool UDiaSkillManagerComponent::AddOwnedVariantBySkillID(int32 SkillID, int32 VariantID)
{
	USkillObject* SkillObj = nullptr;
	for (const auto& Obj : SkillIDMapping)
	{
		if (Obj->GetSkillID() == SkillID)
		{
			SkillObj = Obj;
			break;
		}
	}
	if (!SkillObj)
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::AddOwnedVariantBySkillID: SkillObject is invalid. SkillID: %d"), SkillID);
		return false;
	}

	if (!SkillObj->HasSkillVariantID(VariantID))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::AddOwnedVariantBySkillID: VariantID %d is not valid for SkillID %d"), VariantID, SkillID);
		return false;
	}

	if (SkillObj->HasOwnedVariantID(VariantID))
	{
		return false;
	}

	SkillObj->AddOwnedVariantID(VariantID);
	OnSkillVariantAdded.Broadcast(SkillID, VariantID);
	return true;
}

bool UDiaSkillManagerComponent::TryUnlockSkillVariantByID(int32 SkillID, int32 VariantID)
{
	return AddOwnedVariantBySkillID(SkillID, VariantID);
}

bool UDiaSkillManagerComponent::TryApplySkillVariantByID(int32 SkillID, int32 VariantID)
{
	USkillObject* SkillObj = GetMutableSkillObjectBySkillID(SkillID);
	if (!IsValid(SkillObj))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::TryApplySkillVariantByID: SkillObject is invalid. SkillID: %d"), SkillID);
		return false;
	}

	if (!SkillObj->HasOwnedVariantID(VariantID))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::TryApplySkillVariantByID: VariantID %d is not owned for SkillID %d"), VariantID, SkillID);
		return false;
	}

	SkillObj->SetVariantApplyIDs(VariantID);
	return true;
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

void UDiaSkillManagerComponent::ActiveModifierSkillUseVariants(const FDiaSkillVariantContext& context, const UDiaGameplayAbility* Ability, FSkillModifierRuntime& OutRuntime)
{
	TArray<UDiaSkillVariant*> VariantsToApply;
	MakeSkillVariantsArray(Ability, VariantsToApply);
	FDiaSkillVariantContext& MutableContext = const_cast<FDiaSkillVariantContext&>(context);

	VariantExecutor->ExecuteActiveModifierVariants(VariantsToApply, MutableContext, Ability, OutRuntime);
}

void UDiaSkillManagerComponent::SpawnSkillActorUseVariants(FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability, FDiaSkillSpawnFinishedDelegate OnFinished)
{
	if (!IsValid(Ability))
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: Ability가 유효하지 않습니다."));
		OnFinished.ExecuteIfBound();
		return;
	}

	const USkillObject* SkillObj = Ability->GetSkillObject();
	if (!SkillObj)
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: SkillObject가 유효하지 않습니다."));
		OnFinished.ExecuteIfBound();
		return;
	}

	//여기서는 적용된 variants만 넘겨준다.
	VariantExecutor->ExecuteSpawnVariants(SkillObj->GetVariantApplyIDs(), SkillVariants, context, Ability, OnFinished);
}

void UDiaSkillManagerComponent::HitSkillActorUseVariants(const FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability)
{
	TArray<UDiaSkillVariant*> VariantsToApply;
	MakeSkillVariantsArray(Ability, VariantsToApply);

	FDiaSkillVariantContext& MutableContext = const_cast<FDiaSkillVariantContext&>(context);
	VariantExecutor->ExecuteHitVariants(VariantsToApply, MutableContext, Ability);
}

void UDiaSkillManagerComponent::HitSkillActorUseVariants(const FDiaSkillVariantContext& context, UDiaGameplayAbility* Ability, FSkillHitRuntime& OutRuntime)
{
	TArray<UDiaSkillVariant*> VariantsToApply;
	MakeSkillVariantsArray(Ability, VariantsToApply);

	FDiaSkillVariantContext& MutableContext = const_cast<FDiaSkillVariantContext&>(context);
	VariantExecutor->ExecuteHitVariants(VariantsToApply, MutableContext, Ability, OutRuntime);
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
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::GetSkillVariantFromSkillID: SkillObject가 유효하지 않습니다."));
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

void UDiaSkillManagerComponent::GetOwnedSkillVariantsFromSkillID(const int32 SkillID, OUT TArray<UDiaSkillVariant*>& OutVariants)
{
	const USkillObject* SkillObj = GetSkillObjectBySkillID(SkillID);
	if (!SkillObj)
	{
		UE_LOG(LogARPG, Warning, TEXT("DiaSkillManagerComponent::GetOwnedSkillVariantsFromSkillID: SkillObject is invalid. SkillID: %d"), SkillID);
		return;
	}

	for (const int32 VariantID : SkillObj->GetOwnedVariantIDs())
	{
		if (UDiaSkillVariant* FoundVariant = SkillVariants.FindRef(VariantID))
		{
			OutVariants.Add(FoundVariant);
		}
	}
}

USkillObject* UDiaSkillManagerComponent::GetMutableSkillObjectBySkillID(int32 SkillID)
{
	for (auto& SkillObject : SkillIDMapping)
	{
		if (SkillObject->GetSkillID() == SkillID)
		{
			return SkillObject;
		}
	}
	return nullptr;
}

bool UDiaSkillManagerComponent::TryActivateAbilityBySkillID(int32 SkillID)
{
	const USkillObject* SkillObj = GetSkillObjectBySkillID(SkillID);
	return ActivationService->TryActivateSkill(SkillID, Cast<ADiaBaseCharacter>(GetOwner()), SkillObj);
}

bool UDiaSkillManagerComponent::TryAddSkillLevelBySkillID(int32 SkillID, int32 LevelToAdd)
{
	if(LevelToAdd <= 0)
	{
		return false;
	}

	USkillObject* SkillObj = GetMutableSkillObjectBySkillID(SkillID);
	if (!IsValid(SkillObj))
	{
		return false;
	}

	SkillObj->AddSkillLevel(LevelToAdd);
	OnSkillLevelChanged.Broadcast(SkillID, SkillObj->GetSkillLevel());

	if (FGameplayAbilitySpec* Spec = GetAbilitySpecBySkillID(SkillID))
	{
		Spec->Level = SkillObj->GetSkillLevel();

		if (ADiaBaseCharacter* OwnerCharacter = Cast<ADiaBaseCharacter>(GetOwner()))
		{
			if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
			{
				ASC->MarkAbilitySpecDirty(*Spec);
			}
		}
	}

	return true;
}


