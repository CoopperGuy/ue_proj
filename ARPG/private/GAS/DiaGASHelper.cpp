#include "GAS/DiaGASHelper.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/Abilities/DiaMeleeAbility.h"
#include "GAS/Abilities/DiaProjectileAbility.h"
#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagsManager.h"


//여기서 gameplayeffect를 통해 성질을 부여해야한다.
bool UDiaGASHelper::GrantAbilityFromSkillData(UAbilitySystemComponent* ASC, const FGASSkillData& SkillData, int32 SkillID, FGameplayTag AbilityTag)
{
	if (!ASC)
	{
		return false;
	}

	//해당 태그를 보유중이지 않다면 return false;
	if (!SkillData.AbilityTags.HasTag(AbilityTag))
	{
		return false;
	}

	// Get appropriate ability class based on skill type
	TSubclassOf<UDiaGameplayAbility> AbilityClass = GetAbilityClassFromSkillData(SkillData);
	if (!AbilityClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("No ability class found for skill type: %d"), (int32)SkillData.SkillType);
		return false;
	}

	// Create ability spec with SkillID as InputID
	FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, SkillID);
	
	// Grant the ability
	FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(AbilitySpec);
	
	if (Handle.IsValid())
	{
		// Initialize the ability with skill data
		UDiaGameplayAbility* AbilityInstance = Cast<UDiaGameplayAbility>(ASC->FindAbilitySpecFromHandle(Handle)->GetPrimaryInstance());
		if (AbilityInstance)
		{
			AbilityInstance->InitializeWithSkillData(SkillData);
		}

		UE_LOG(LogTemp, Log, TEXT("Granted ability for skill ID %d"), SkillID);
		return true;
	}

	return false;
}

bool UDiaGASHelper::RevokeAbilityBySkillID(UAbilitySystemComponent* ASC, int32 SkillID)
{
	if (!ASC)
	{
		return false;
	}

	FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecBySkillID(ASC, SkillID);
	if (AbilitySpec)
	{
		ASC->ClearAbility(AbilitySpec->Handle);
		UE_LOG(LogTemp, Log, TEXT("Revoked ability for skill ID %d"), SkillID);
		return true;
	}

	return false;
}

bool UDiaGASHelper::TryActivateAbilityBySkillID(UAbilitySystemComponent* ASC, int32 SkillID)
{
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaGASHelper::TryActivateAbilityBySkillID: Invalid ASC"));
		return false;
	}

	UE_LOG(LogTemp, Verbose, TEXT("TryActivateAbilityBySkillID: SkillID=%d"), SkillID);

	// 일반적인 스킬 ID 처리
	FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecBySkillID(ASC, SkillID);
	if (AbilitySpec)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Found Spec: Ability=%s, InputID=%d, Active=%s"),
			*GetNameSafe(AbilitySpec->Ability), AbilitySpec->InputID, AbilitySpec->IsActive() ? TEXT("true") : TEXT("false"));

		const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
		UE_LOG(LogTemp, Verbose, TEXT("ActorInfo: Avatar=%s, Owner=%s"),
			*GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr),
			*GetNameSafe(ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr));

		const bool bActivated = ASC->TryActivateAbility(AbilitySpec->Handle);
		UE_LOG(LogTemp, Log, TEXT("TryActivateAbility result: %s"), bActivated ? TEXT("true") : TEXT("false"));
		return bActivated;
	}

	UE_LOG(LogTemp, Warning, TEXT("DiaGASHelper::TryActivateAbilityBySkillID: No ability found for SkillID %d"), SkillID);
	return false;
}

bool UDiaGASHelper::CanActivateAbilityBySkillID(UAbilitySystemComponent* ASC, int32 SkillID)
{
	if (!ASC)
	{
		return false;
	}

	FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecBySkillID(ASC, SkillID);
	if (AbilitySpec && AbilitySpec->Ability)
	{
		return !AbilitySpec->IsActive();
	}

	return false;
}

FGameplayAbilitySpec* UDiaGASHelper::GetAbilitySpecBySkillID(UAbilitySystemComponent* ASC, int32 SkillID)
{
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

float UDiaGASHelper::GetCooldownRemainingBySkillID(UAbilitySystemComponent* ASC, int32 SkillID)
{
	if (!ASC)
	{
		return 0.0f;
	}

	FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecBySkillID(ASC, SkillID);
	if (!AbilitySpec || !AbilitySpec->Ability)
	{
		return 0.0f;
	}

	return 0.0f;
}

TSubclassOf<UDiaGameplayAbility> UDiaGASHelper::GetAbilityClassFromSkillData(const FGASSkillData& SkillData)
{
	if (SkillData.AbilityClass)
	{
		return SkillData.AbilityClass;
	}
	switch (SkillData.SkillType)
	{
	case EGASSkillType::MeleeAttack:
		return UDiaMeleeAbility::StaticClass();
	case EGASSkillType::Magic:
		return UDiaProjectileAbility::StaticClass();
	case EGASSkillType::RangedAttack:
		return UDiaProjectileAbility::StaticClass();
	case EGASSkillType::Passive:
		// Passive skills would need their own ability class
		// For now, return base ability class
		return UDiaGameplayAbility::StaticClass();
	default:
		return UDiaGameplayAbility::StaticClass();
	}
}