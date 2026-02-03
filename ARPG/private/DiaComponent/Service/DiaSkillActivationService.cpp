// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Service/DiaSkillActivationService.h"
#include "DiaBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaComponent/Skill/SkillObject.h"

bool UDiaSkillActivationService::TryActivateSkill(int32 SkillID, ADiaBaseCharacter* Character, const USkillObject* SkillObj)
{
	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiaSkillActivationService::TryActivateSkill: Invalid Character"));
		return false;
	}

	UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
	if (!IsValid(ASC))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDiaSkillActivationService::TryActivateSkill: Invalid ASC"));
		return false;
	}

	// SkillID를 InputID로 사용하여 AbilitySpec 찾기
	FGameplayAbilitySpec* AbilitySpec = nullptr;
	for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.InputID == SkillID)
		{
			AbilitySpec = &Spec;
			break;
		}
	}

	if (!AbilitySpec)
	{
		return false;
	}

	// SkillObject가 제공된 경우, Ability 인스턴스에 설정
	if (SkillObj)
	{
		UDiaGameplayAbility* AbilityInstance = Cast<UDiaGameplayAbility>(AbilitySpec->GetPrimaryInstance());
		if (AbilityInstance)
		{
			AbilityInstance->SetSkillObject(SkillObj);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UDiaSkillActivationService::TryActivateSkill: AbilityInstance를 가져올 수 없습니다. SkillID: %d"), SkillID);
		}
	}

	return ASC->TryActivateAbility(AbilitySpec->Handle);
}