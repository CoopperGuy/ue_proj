// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaComponent/Skill/Executor/DiaSkillVariantSpawnExecutor.h"

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
}

void UDiaSkillManagerComponent::LoadJobSKillDataFromTable(EJobType JobType)
{
	UGameInstance* Instance = GetWorld()->GetGameInstance();
	if(IsValid(Instance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::LoadJobSKillDataFromTable: GameInstance가 유효하지 않습니다."));
		return;
	}
	UJobSkillSetSubSystem* JobSkillSubSystem = Instance->GetSubsystem<UJobSkillSetSubSystem>();
	if(IsValid(JobSkillSubSystem) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::LoadJobSKillDataFromTable: UJobSkillSetSubSystem가 유효하지 않습니다."));
		return;
	}
	UGASSkillManager* GASSkillManager = Instance->GetSubsystem<UGASSkillManager>();
	if (IsValid(GASSkillManager) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::LoadJobSKillDataFromTable: JobSkillSubSystem가 유효하지 않습니다."));
		return;
	}

	CurrentJobSkillSet = JobSkillSubSystem->GetJobSkillSet(JobType);

	for(const int32 SkillID : CurrentJobSkillSet.SkillIDs)
	{
		USkillObject* NewSkillObject = NewObject<USkillObject>(this);
		NewSkillObject->SetSkillID(SkillID);

		const FGASSkillData* GASData = GASSkillManager->GetSkillDataPtr(SkillID);
		NewSkillObject->SetSkillVariantIDs(GASData->VariantIDs);

		SkillIDMapping.Add(NewSkillObject);

		//게임에서 사용할 variant 객체들 미리 생성
		for(const int32 VariantID : GASData->VariantIDs)
		{
			if (SkillVariants.Find(VariantID) == nullptr)
			{
				UDiaSkillVariant* NewDiaSkillVariant = NewObject<UDiaSkillVariant>(this);
				NewDiaSkillVariant->InitializeVariant(VariantID);
				SkillVariants.Add(VariantID, NewDiaSkillVariant);
			}
		}
	}
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
	if (!IsValid(Ability))
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: Ability가 유효하지 않습니다."));
		return;
	}

	if (!context.SkillActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: SkillActorClass가 유효하지 않습니다."));
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
	
	UDiaSkillVariantSpawnExecutor* SpawnExecutor = NewObject<UDiaSkillVariantSpawnExecutor>(this);

	if (!IsValid(SpawnExecutor))
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::SpawnSkillActorUseVariants: SpawnExecutor가 유효하지 않습니다."));
		return;
	}
	TArray<int32> VariantIDs = SkillObj->GetSkillVariantIDs();
	TArray<UDiaSkillVariant*> VariantsToApply;
	constexpr int32 EstimatedNumVariants = 6;
	VariantsToApply.Reserve(EstimatedNumVariants);
	if (VariantIDs.Num() > 0)
	{
		for(const int32& VariantID : VariantIDs)
		{
			if (UDiaSkillVariant* FoundVariant = SkillVariants.FindRef(VariantID))
			{
				VariantsToApply.Add(FoundVariant);
			}
		}
	}

	//이거 나중에 array에 하나로 넣어서 한번에 실행하면 좋을거같은데? context를 모아두는거임.
	SpawnExecutor->ExecuteEffect(VariantsToApply, context, Ability);

}

bool UDiaSkillManagerComponent::TryActivateAbilityBySkillID(int32 SkillID)
{
	ADiaBaseCharacter* OwnerCharacter = Cast<ADiaBaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::TryActivateAbilityBySkillID: Invalid Owner"));
		return false;
	}

	UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::TryActivateAbilityBySkillID: Invalid ASC"));
		return false;
	}

	// 일반적인 스킬 ID 처리
	FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecBySkillID(SkillID);
	if (AbilitySpec)
	{
		// ★ 중요: TryActivateAbility 호출 전에 SkillObject를 설정해야 함
		// ActivateAbility 내부에서 SpawnSkillActorUseVariants가 호출되기 때문
		// ★ GAS는 InstancedPerActor이므로 GetPrimaryInstance()를 사용해야 함
		if(const USkillObject* SkillObj = GetSkillObjectBySkillID(SkillID))
		{
			// Ability 인스턴스 가져오기 (InstancedPerActor이므로)
			UDiaGameplayAbility* AbilityInstance = Cast<UDiaGameplayAbility>(AbilitySpec->GetPrimaryInstance());
			if (AbilityInstance)
			{
				AbilityInstance->SetSkillObject(SkillObj);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::TryActivateAbilityBySkillID: AbilityInstance를 가져올 수 없습니다. SkillID: %d"), SkillID);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DiaSkillManagerComponent::TryActivateAbilityBySkillID: SkillObject를 찾을 수 없습니다. SkillID: %d"), SkillID);
		}

		const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
		const bool bActivated = ASC->TryActivateAbility(AbilitySpec->Handle);

		return bActivated;
	}

	return false;
}


