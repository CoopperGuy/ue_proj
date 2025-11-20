#include "GAS/DiaAttributeSet.h"
#include "GameplayEffect.h"
#include "System/CharacterManager.h"
#include "GAS/DiaGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "DiaBaseCharacter.h"

TMap<FGameplayTag, FGameplayAttribute> UDiaAttributeSet::AttributeTagMap;

UDiaAttributeSet::UDiaAttributeSet()
{
	// Initialize default values
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitMana(50.0f);
	InitMaxMana(50.0f);
	InitAttackPower(10.0f);
	InitDefense(5.0f);
	InitMovementSpeed(600.0f);
	InitExp(0.0f);
	InitMaxExp(100.0f);

	
}


void UDiaAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp attributes to valid ranges
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetAttackPowerAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetDefenseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetMovementSpeedAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetExpAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxExp());
	}
	else if (Attribute == GetMaxExpAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}
}

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

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		// Handle incoming damage
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.0f);

		if (LocalIncomingDamage > 0)
		{
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
		}
	}
	else if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
	{
		// Handle incoming healing
		const float LocalIncomingHealing = GetIncomingHealing();
		SetIncomingHealing(0.0f);

		if (LocalIncomingHealing > 0)
		{
			// Apply healing to health
			const float NewHealth = GetHealth() + LocalIncomingHealing;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Clamp health
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
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
	}
}

void UDiaAttributeSet::InitializeCharacterAttributes(FName CharacterID, int32 Level)
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
	SetMaxHealth(CharacterManager->CalculateMaxHPForLevel(CharacterInfo, Level));
	SetHealth(GetMaxHealth());
	SetMaxMana(CharacterManager->CalculateMaxMPForLevel(CharacterInfo, Level));
	SetMana(GetMaxMana());
	SetAttackPower(CharacterInfo->BaseAttackPower + (Level - 1) * CharacterInfo->AttackPowerPerLevel);
	SetDefense(CharacterInfo->BaseDefense + (Level - 1) * CharacterInfo->DefensePerLevel);
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

bool UDiaAttributeSet::TranslateAttributeTagToAttrivute(const FGameplayTag& AttributeTag, FGameplayAttribute& OutAttribute)
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

void UDiaAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
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

void UDiaAttributeSet::MakeAttributeTagMap()
{
	const auto& Tags = FDiaGameplayTags::Get();

	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Health")), GetHealthAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.MaxHealth")), GetMaxHealthAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Mana")), GetManaAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.MaxMana")), GetMaxManaAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Exp")), GetExpAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.MaxExp")), GetMaxExpAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Strength")), GetStrengthAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Dexterity")), GetDexterityAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Intelligence")), GetIntelligenceAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.AttackPower")), GetAttackPowerAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Defense")), GetDefenseAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.MovementSpeed")), GetMovementSpeedAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.IncomingDamage")), GetIncomingDamageAttribute());
	AttributeTagMap.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.IncomingHealing")), GetIncomingHealingAttribute());

}
