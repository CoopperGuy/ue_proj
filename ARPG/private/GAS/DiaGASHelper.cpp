#include "GAS/DiaGASHelper.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/Abilities/DiaMeleeAbility.h"
#include "GAS/Abilities/DiaProjectileAbility.h"
#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "AbilitySystemComponent.h"


//여기서 gameplayeffect를 통해 성질을 부여해야한다.
bool UDiaGASHelper::GrantAbilityFromSkillData(UAbilitySystemComponent* ASC, const FGASSkillData& SkillData, int32 SkillID, FGameplayTag AbilityTag)
{
	if (!ASC)
	{
		return false;
	}

	// 태그 체크: AbilityTag가 유효하고 SkillData에 해당 태그가 없으면 경고만 출력
	if (AbilityTag.IsValid() && !SkillData.AbilityTags.HasTag(AbilityTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("GrantAbilityFromSkillData: SkillID %d doesn't have required tag %s, granting anyway"), 
			SkillID, *AbilityTag.ToString());
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
	
	// ★ SkillID 기반 쿨다운 태그 조회 (INI 파일에 미리 등록되어 있어야 함)
	FString CooldownTagName = FString::Printf(TEXT("GASData.CoolDown.%d"), SkillID);
	FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(*CooldownTagName), false);
	
	if (!CooldownTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("GrantAbilityFromSkillData: Cooldown tag not found for SkillID %d. Add 'GASData.CoolDown.%d' to DefaultGameplayTags.ini"), 
			SkillID, SkillID);
	}
	
	// Spec의 SetByCallerTagMagnitudes에 쿨다운 태그 저장 (나중에 참조용)
	if (CooldownTag.IsValid())
	{
		AbilitySpec.SetByCallerTagMagnitudes.Add(CooldownTag, SkillData.CooldownDuration);
	}
	
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

		//UE_LOG(LogTemp, Log, TEXT("Granted ability for skill ID %d with cooldown tag: %s"), SkillID, *CooldownTagName);
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

	//UE_LOG(LogTemp, Verbose, TEXT("TryActivateAbilityBySkillID: SkillID=%d"), SkillID);

	// 일반적인 스킬 ID 처리
	FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecBySkillID(ASC, SkillID);
	if (AbilitySpec)
	{
		//UE_LOG(LogTemp, Verbose, TEXT("Found Spec: Ability=%s, InputID=%d, Active=%s"),
		//	*GetNameSafe(AbilitySpec->Ability), AbilitySpec->InputID, AbilitySpec->IsActive() ? TEXT("true") : TEXT("false"));

		const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
		//UE_LOG(LogTemp, Verbose, TEXT("ActorInfo: Avatar=%s, Owner=%s"),
		//	*GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr),
		//	*GetNameSafe(ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr));

		const bool bActivated = ASC->TryActivateAbility(AbilitySpec->Handle);
		//UE_LOG(LogTemp, Log, TEXT("TryActivateAbility result: %s"), bActivated ? TEXT("true") : TEXT("false"));
		return bActivated;
	}

	//UE_LOG(LogTemp, Warning, TEXT("DiaGASHelper::TryActivateAbilityBySkillID: No ability found for SkillID %d"), SkillID);
	return false;
}

bool UDiaGASHelper::CanActivateAbilityBySkillID(UAbilitySystemComponent* ASC, int32 SkillID)
{
	if (!ASC)
	{
		//UE_LOG(LogTemp, Warning, TEXT("[CanActivateAbility] ASC is null for SkillID: %d"), SkillID);
		return false;
	}

	FGameplayAbilitySpec* AbilitySpec = GetAbilitySpecBySkillID(ASC, SkillID);
	if (!AbilitySpec || !AbilitySpec->Ability)
	{
		//UE_LOG(LogTemp, Warning, TEXT("[CanActivateAbility] No ability spec found for SkillID: %d"), SkillID);
		return false;
	}

	const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
	if (!ActorInfo)
	{
		//UE_LOG(LogTemp, Warning, TEXT("[CanActivateAbility] ActorInfo is null for SkillID: %d"), SkillID);
		return false;
	}

	AActor* OwnerActor = ActorInfo->OwnerActor.Get();
	FString OwnerName = OwnerActor ? OwnerActor->GetName() : TEXT("Unknown");
	FString AbilityName = GetNameSafe(AbilitySpec->Ability);

	// 1. 이미 활성화 중이면 false
	if (AbilitySpec->IsActive())
	{
		//UE_LOG(LogTemp, Warning, TEXT("[CanActivateAbility] [%s] SkillID: %d, Ability: %s - Already Active"), *OwnerName, SkillID, *AbilityName);
		return false;
	}

	// 2. Cooldown 체크 - Ability의 CheckCooldown 사용
	bool bCanUseCooldown = AbilitySpec->Ability->CheckCooldown(AbilitySpec->Handle, ActorInfo);
	if (!bCanUseCooldown)
	{
		//UE_LOG(LogTemp, Warning, TEXT("[CanActivateAbility] [%s] SkillID: %d, Ability: %s - On Cooldown"), 
		//	*OwnerName, SkillID, *AbilityName);
		return false;
	}

	// 3. Cost 체크 - false를 반환하면 비용을 지불할 수 없음
	bool bCanPayCost = AbilitySpec->Ability->CheckCost(AbilitySpec->Handle, ActorInfo);
	if (!bCanPayCost)
	{
		//UE_LOG(LogTemp, Warning, TEXT("[CanActivateAbility] [%s] SkillID: %d, Ability: %s - Cannot Pay Cost"), *OwnerName, SkillID, *AbilityName);
		return false;
	}

	//UE_LOG(LogTemp, Log, TEXT("[CanActivateAbility] [%s] SkillID: %d, Ability: %s - Can Activate"), *OwnerName, SkillID, *AbilityName);
	return true;
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