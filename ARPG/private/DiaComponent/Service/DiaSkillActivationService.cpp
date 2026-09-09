// Fill out your copyright notice in the Description page of Project Settings.

#include "DiaComponent/Service/DiaSkillActivationService.h"

#include "AbilitySystemComponent.h"
#include "DiaBaseCharacter.h"
#include "GAS/DiaGASHelper.h"
#include "GAS/DiaGameplayAbility.h"
#include "GameplayAbilitySpec.h"
#include "Logging/ARPGLogChannels.h"

bool UDiaSkillActivationService::CanActivateSkill(
	int32 SkillID,
	ADiaBaseCharacter* Character,
	const USkillObject* SkillObj)
{
	if (!IsValid(Character))
	{
		return false;
	}

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		return false;
	}

	FGameplayAbilitySpec* AbilitySpec = UDiaGASHelper::GetAbilitySpecBySkillID(ASC, SkillID);
	if (!PrepareAbilityRuntime(AbilitySpec, Character, SkillObj))
	{
		return false;
	}

	return UDiaGASHelper::CanActivateAbilityBySkillID(ASC, SkillID);
}

bool UDiaSkillActivationService::TryActivateSkill(
	int32 SkillID,
	ADiaBaseCharacter* Character,
	const USkillObject* SkillObj)
{
	if (!IsValid(Character))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillActivationService::TryActivateSkill: Invalid Character"));
		return false;
	}

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogARPG, Warning, TEXT("UDiaSkillActivationService::TryActivateSkill: Invalid ASC"));
		return false;
	}

	FGameplayAbilitySpec* AbilitySpec = UDiaGASHelper::GetAbilitySpecBySkillID(ASC, SkillID);
	if (!PrepareAbilityRuntime(AbilitySpec, Character, SkillObj))
	{
		return false;
	}

	return ASC->TryActivateAbility(AbilitySpec->Handle);
}

bool UDiaSkillActivationService::PrepareAbilityRuntime(
	FGameplayAbilitySpec* AbilitySpec,
	ADiaBaseCharacter* Character,
	const USkillObject* SkillObj) const
{
	if (!AbilitySpec || !IsValid(Character))
	{
		return false;
	}

	UDiaGameplayAbility* AbilityInstance = Cast<UDiaGameplayAbility>(AbilitySpec->GetPrimaryInstance());
	if (!IsValid(AbilityInstance))
	{
		UE_LOG(
			LogARPG,
			Warning,
			TEXT("UDiaSkillActivationService: Primary ability instance is unavailable. InputID: %d"),
			AbilitySpec->InputID);
		return false;
	}

	AbilityInstance->SetSkillObject(SkillObj);
	AbilityInstance->ActiveModifierEffect(Character);
	return true;
}
