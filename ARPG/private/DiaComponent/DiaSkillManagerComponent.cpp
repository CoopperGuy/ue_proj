// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"

#include "GAS/DiaGameplayAbility.h"

#include "System/JobSkillSetSubSystem.h"
#include "DiaBaseCharacter.h"

#include "AbilitySystemComponent.h"

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

	CurrentJobSkillSet = JobSkillSubSystem->GetJobSkillSet(JobType);

	for(const int32 SkillID : CurrentJobSkillSet.SkillIDs)
	{
		UE_LOG(LogTemp, Log, TEXT("DiaSkillManagerComponent: 로드된 스킬 ID - %d"), SkillID);
		USkillObject* NewSkillObject = NewObject<USkillObject>(this);
		NewSkillObject->SetSkillID(SkillID);
		SkillIDMapping.Add(NewSkillObject);
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
		const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
		const bool bActivated = ASC->TryActivateAbility(AbilitySpec->Handle);
		if (bActivated)
		{
			if(const USkillObject* SkillObj = GetSkillObjectBySkillID(SkillID))
			{
				if (UDiaGameplayAbility* const Ability = Cast<UDiaGameplayAbility>(AbilitySpec->Ability))
				{
					Ability->SetSkillObject(SkillObj);
				}
			}
		}

		return bActivated;
	}

	return false;
}


