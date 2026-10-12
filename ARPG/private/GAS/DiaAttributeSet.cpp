#include "GAS/DiaAttributeSet.h"
#include "GameplayEffect.h"
#include "System/CharacterManager.h"
#include "GAS/DiaGameplayTags.h"
#include "GAS/DiaEquipmentCombat.h"
#include "GAS/Effects/Exec_Damage.h"
#include "Types/DiaEquipmentOptionRules.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "DiaBaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Logging/ARPGLogChannels.h"


TMap<FGameplayTag, FGameplayAttribute> UDiaAttributeSet::AttributeTagMap;

UDiaAttributeSet::UDiaAttributeSet()
{
	// Initialize default values
	InitLevel(1.0f);
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitMana(50.0f);
	InitMaxMana(50.0f);
	InitAttackPower(10.0f);
	InitDefense(5.0f);
	InitMovementSpeed(600.0f);
	InitExp(0.0f);
	InitMaxExp(100.0f);
	InitCooldownReduction(0.0f);
	InitCritChance(0.f);
	InitCritDamage(150.f);
}


void UDiaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 음수 가능하도록 변경 (디버프도 있으니까)
	NewValue = NewValue;

	// 상한선이 있는 Attribute만 Clamp
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Min(NewValue, GetMaxHealth());
	}
	else if (Attribute == GetLevelAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Min(NewValue, GetMaxMana());
	}
	else if (Attribute == GetExpAttribute())
	{
		NewValue = FMath::Min(NewValue, GetMaxExp());
	}
	else if (Attribute == GetAttackPowerAttribute()
		|| Attribute == GetManaAttribute()
		|| Attribute == GetExpAttribute()
		|| Attribute == GetCooldownReductionAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}	
}

//이게 어떤 GE 때문에 일어났는지 알 수 있느 훅 
void UDiaAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();

	AActor* Owner = GetOwningActor();
	ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(Owner);

	AActor* Causer = Context.GetInstigator();
	ADiaBaseCharacter* CauserCharacter = Cast<ADiaBaseCharacter>(Causer);

	float DeltaValue = 0;
	if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
	{
		DeltaValue = Data.EvaluatedData.Magnitude;
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
        // Reconstruct pre-hit health before clamping away overkill. Only damage executions
        // can trigger equipment: healing, costs and direct periodic modifiers cannot recurse.
        const float HealthBefore = FMath::Clamp(GetHealth() - DeltaValue, 0.f, GetMaxHealth());
        SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
        const bool bDamageExecution = Data.EffectSpec.Def && Data.EffectSpec.Def->Executions.ContainsByPredicate(
            [](const FGameplayEffectExecutionDefinition& Execution)
            {
                return Execution.CalculationClass && Execution.CalculationClass->IsChildOf(UExec_Damage::StaticClass());
            });
        if (DeltaValue < 0.f && HealthBefore > 0.f && bDamageExecution
            && IsValid(Character) && !Character->GetIsDead())
        {
            DiaEquipmentCombat::OnConfirmedHit(Source, GetOwningAbilitySystemComponent(),
                DiaEquipmentOptionRules::ConfirmedHealthLoss(HealthBefore, GetHealth()), GetHealth() <= 0.f);
        }
        if (IsValid(Character)) Character->OnRecieveDamage(DeltaValue);
		if (GetHealth() <= 0.f)
		{
			if (IsValid(Character))
			{
				Character->Die(CauserCharacter);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		// Clamp mana
		SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	}
	else if(Data.EvaluatedData.Attribute == GetExpAttribute())
	{
		if (GetExp() >= GetMaxExp())
		{
			//레벨업 로직
			if (IsValid(Character))
			{
				Character->OnLevelUp();
				SetExp(FMath::Clamp(GetExp() - GetMaxExp(), 0.0f, GetMaxExp()));
			}
		}
		else
		{
			SetExp(FMath::Clamp(GetExp(), 0.0f, GetMaxExp()));
		}
	}else    if (Data.EvaluatedData.Attribute == GetAttackPowerAttribute())
	{
		UE_LOG(LogARPGAttribute, Verbose,
			TEXT("PostGameplayEffectExecute AttackPower: Delta=%f, NewValue=%f"),
			DeltaValue, GetAttackPower());
	}
}

void UDiaAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	// Removing equipment is an aggregator change, not an instant-effect execution.
	if (Attribute == GetMaxHealthAttribute() && GetHealth() > NewValue)
	{
		SetHealth(FMath::Max(0.0f, NewValue));
	}
	if (Attribute == GetMaxManaAttribute() && GetMana() > NewValue)
	{
		SetMana(FMath::Max(0.0f, NewValue));
	}
	// 추가적인 후처리 로직이 필요한 경우 여기에 작성
	UE_LOG(LogARPGAttribute, Warning,
		TEXT("PostAttributeChange Attribute: %s, OldValue=%f, NewValue=%f"),
		*Attribute.GetName(), OldValue, NewValue);

	if (Attribute == GetMovementSpeedAttribute())
	{
		AActor* AvatarActor = nullptr;
		if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
		{
			AvatarActor = ASC->GetAvatarActor();
		}
		if (!AvatarActor)
		{
			AvatarActor = GetOwningActor();
		}

		if (ADiaBaseCharacter* Character = Cast<ADiaBaseCharacter>(AvatarActor))
		{
			if (UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement())
			{
				MovementComponent->MaxWalkSpeed = FMath::Max(NewValue, 0.f);
				UE_LOG(LogARPGAttribute, Warning,
					TEXT("Apply MovementSpeed to MaxWalkSpeed: %s, MaxWalkSpeed=%f"),
					*GetNameSafe(Character), MovementComponent->MaxWalkSpeed);
			}
		}
	}
}

void UDiaAttributeSet::InitializeCharacterAttributes(FName CharacterID, int32 InLevel)
{
	// 게임 인스턴스에서 캐릭터 매니저 가져오기
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)
	{
		return;
	}

	UCharacterManager* CharacterManager = GI->GetSubsystem<UCharacterManager>();
	if (!CharacterManager)
	{
		return;
	}

	const FCharacterInfo* CharacterInfo = CharacterManager->GetCharacterInfo(CharacterID);
	if (!CharacterInfo)
	{
		return;
	}

	// 체력/마나 초기화
	SetLevel(FMath::Max(InLevel, 1));
	SetMaxHealth(CharacterManager->CalculateMaxHPForLevel(CharacterInfo, InLevel));
	SetHealth(GetMaxHealth());
	SetMaxMana(CharacterManager->CalculateMaxMPForLevel(CharacterInfo, InLevel));
	SetMana(GetMaxMana());
	SetAttackPower(CharacterInfo->BaseAttackPower + (InLevel - 1) * CharacterInfo->AttackPowerPerLevel);
	SetDefense(CharacterInfo->BaseDefense + (InLevel - 1) * CharacterInfo->DefensePerLevel);
	SetMovementSpeed(600.0f); // 기본 이동 속도 설정
}

void UDiaAttributeSet::InitializeMonsterAttributes(const FMonsterInfo& MonsterInfo)
{
	SetMaxHealth(MonsterInfo.MaxHP);
	SetHealth(MonsterInfo.MaxHP);
	SetMaxMana(MonsterInfo.MaxMP);
	SetMana(MonsterInfo.MaxMP);
	SetAttackPower(MonsterInfo.Attack);
	SetDefense(MonsterInfo.Defense);
	SetMovementSpeed(600.0f);
	SetExp(0.f);
	SetMaxExp(MonsterInfo.Exp);
}

bool UDiaAttributeSet::TranslateAttributeTagToAttribute(const FGameplayTag& AttributeTag, FGameplayAttribute& OutAttribute)
{
	if (AttributeTagMap.Num() == 0)
	{
		MakeAttributeTagMap();
	}

	const FGameplayAttribute* Found = AttributeTagMap.Find(AttributeTag);
	if (Found)
	{
		OutAttribute = *Found;
		return true;
	}

	return false;
}

void UDiaAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, 
	const FGameplayAttributeData& MaxAttribute, 
	float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UDiaAttributeSet::AdjustAttributeForValChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, 
	float NewValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (AbilityComp)
	{
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = FMath::Clamp(NewValue, 0.f, CurrentMaxValue) - CurrentValue;
		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UDiaAttributeSet::MakeAttributeTagMap()
{
	if (AttributeTagMap.Num() > 0)
	{
		return;
	}

	AttributeTagMap.Reserve(22);

	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_Level.GetTag(), GetLevelAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_Health.GetTag(), GetHealthAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_MaxHealth.GetTag(), GetMaxHealthAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_Mana.GetTag(), GetManaAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_MaxMana.GetTag(), GetMaxManaAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_Exp.GetTag(), GetExpAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_MaxExp.GetTag(), GetMaxExpAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_Strength.GetTag(), GetStrengthAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_Dexterity.GetTag(), GetDexterityAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_Intelligence.GetTag(), GetIntelligenceAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_AttackPower.GetTag(), GetAttackPowerAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_Defense.GetTag(), GetDefenseAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_DamageIncreaseOption.GetTag(), GetDamageIncreaseOptionAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_MovementSpeed.GetTag(), GetMovementSpeedAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_CritChance.GetTag(), GetCritChanceAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_CritDamage.GetTag(), GetCritDamageAttribute());
	AttributeTagMap.Add(DiaGameplayTags::AttributeSet_CooldownReduction.GetTag(), GetCooldownReductionAttribute());

	AttributeTagMap.Add(DiaGameplayTags::ItemOption_CooldownReduction.GetTag(), GetCooldownReductionAttribute());
	AttributeTagMap.Add(DiaGameplayTags::ItemOption_MaxHealth.GetTag(), GetMaxHealthAttribute());
	AttributeTagMap.Add(DiaGameplayTags::ItemOption_MovementSpeed.GetTag(), GetMovementSpeedAttribute());
	AttributeTagMap.Add(DiaGameplayTags::ItemOption_CritChance.GetTag(), GetCritChanceAttribute());
	AttributeTagMap.Add(DiaGameplayTags::ItemOption_DamageIncreaseOption.GetTag(), GetDamageIncreaseOptionAttribute());
	AttributeTagMap.Add(DiaGameplayTags::ItemOption_AttackPower.GetTag(), GetAttackPowerAttribute());
	AttributeTagMap.Add(DiaGameplayTags::ItemOption_Defense.GetTag(), GetDefenseAttribute());
	AttributeTagMap.Add(DiaGameplayTags::ItemOption_MaxMana.GetTag(), GetMaxManaAttribute());
	AttributeTagMap.Add(DiaGameplayTags::ItemOption_CritDamage.GetTag(), GetCritDamageAttribute());

}
