// Fill out your copyright notice in the Description page of Project Settings.

#include "DiaComponent/DiaStatComponent.h"
#include "DiaBaseCharacter.h"

UDiaStatComponent::UDiaStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDiaStatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDiaStatComponent::InitializeFromData(const FMonsterInfo& MonsterInfo)
{
	CombatStats = MonsterInfo;
	CharacterData = MonsterInfo;
	
	// 체력과 마나를 최대치로 설정
	CharacterData.Health = CharacterData.MaxHealth;
	CharacterData.Mana = CharacterData.MaxMana;
	
	// 레벨 초기화 (몬스터는 경험치 획득량만 사용)
	LevelData.CurrentLevel = 1;
	LevelData.CurrentExp = 0.f;
	LevelData.MaxExp = LevelData.GetRequiredExpForLevel(LevelData.CurrentLevel + 1);
}

void UDiaStatComponent::TakeDamage(float DamageAmount)
{
	if (DamageAmount <= 0.0f || IsDead())
	{
		return;
	}
	
	float OldHealth = CharacterData.Health;
	CharacterData.Health = FMath::Max(0.0f, CharacterData.Health - DamageAmount);
	
	// 체력이 실제로 변경된 경우에만 이벤트 발생
	if (CharacterData.Health != OldHealth)
	{
		OnHealthChanged.Broadcast(CharacterData.Health, CharacterData.MaxHealth);
		
		UE_LOG(LogTemp, Log, TEXT("%s took %f damage, Health: %f/%f"), 
			*GetOwner()->GetName(), DamageAmount, CharacterData.Health, CharacterData.MaxHealth);
	}
}

void UDiaStatComponent::RestoreHealth(float HealthAmount)
{
	if (HealthAmount <= 0.0f || IsDead())
	{
		return;
	}
	
	float OldHealth = CharacterData.Health;
	CharacterData.Health = FMath::Min(CharacterData.MaxHealth, CharacterData.Health + HealthAmount);
	
	// 체력이 실제로 변경된 경우에만 이벤트 발생
	if (CharacterData.Health != OldHealth)
	{
		OnHealthChanged.Broadcast(CharacterData.Health, CharacterData.MaxHealth);
		
		UE_LOG(LogTemp, Log, TEXT("%s restored %f health, Health: %f/%f"), 
			*GetOwner()->GetName(), HealthAmount, CharacterData.Health, CharacterData.MaxHealth);
	}
}

void UDiaStatComponent::SetMaxHealth(float NewMaxHealth)
{
	if (NewMaxHealth <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid MaxHealth value: %f"), NewMaxHealth);
		return;
	}
	
	float OldMaxHealth = CharacterData.MaxHealth;
	CharacterData.MaxHealth = NewMaxHealth;
	
	// 현재 체력이 새로운 최대 체력을 초과하는 경우 조정
	if (CharacterData.Health > CharacterData.MaxHealth)
	{
		CharacterData.Health = CharacterData.MaxHealth;
	}
	
	if (OldMaxHealth != CharacterData.MaxHealth)
	{
		OnHealthChanged.Broadcast(CharacterData.Health, CharacterData.MaxHealth);
	}
}

void UDiaStatComponent::ConsumeMana(float ManaAmount)
{
	if (ManaAmount <= 0.0f)
	{
		return;
	}
	
	float OldMana = CharacterData.Mana;
	CharacterData.Mana = FMath::Max(0.0f, CharacterData.Mana - ManaAmount);
	
	// 마나가 실제로 변경된 경우에만 이벤트 발생
	if (CharacterData.Mana != OldMana)
	{
		OnManaChanged.Broadcast(CharacterData.Mana, CharacterData.MaxMana);
		
		UE_LOG(LogTemp, Log, TEXT("%s consumed %f mana, Mana: %f/%f"), 
			*GetOwner()->GetName(), ManaAmount, CharacterData.Mana, CharacterData.MaxMana);
	}
}

void UDiaStatComponent::RestoreMana(float ManaAmount)
{
	if (ManaAmount <= 0.0f)
	{
		return;
	}
	
	float OldMana = CharacterData.Mana;
	CharacterData.Mana = FMath::Min(CharacterData.MaxMana, CharacterData.Mana + ManaAmount);
	
	// 마나가 실제로 변경된 경우에만 이벤트 발생
	if (CharacterData.Mana != OldMana)
	{
		OnManaChanged.Broadcast(CharacterData.Mana, CharacterData.MaxMana);
		
		UE_LOG(LogTemp, Log, TEXT("%s restored %f mana, Mana: %f/%f"), 
			*GetOwner()->GetName(), ManaAmount, CharacterData.Mana, CharacterData.MaxMana);
	}
}

void UDiaStatComponent::SetMaxMana(float NewMaxMana)
{
	if (NewMaxMana <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid MaxMana value: %f"), NewMaxMana);
		return;
	}
	
	float OldMaxMana = CharacterData.MaxMana;
	CharacterData.MaxMana = NewMaxMana;
	
	// 현재 마나가 새로운 최대 마나를 초과하는 경우 조정
	if (CharacterData.Mana > CharacterData.MaxMana)
	{
		CharacterData.Mana = CharacterData.MaxMana;
	}
	
	if (OldMaxMana != CharacterData.MaxMana)
	{
		OnManaChanged.Broadcast(CharacterData.Mana, CharacterData.MaxMana);
	}
}

bool UDiaStatComponent::HasEnoughMana(float ManaAmount) const
{
	return CharacterData.Mana >= ManaAmount;
}

void UDiaStatComponent::AddExperience(float ExpAmount)
{
	if (ExpAmount <= 0.0f)
	{
		return;
	}
	
	float OldExp = LevelData.CurrentExp;
	LevelData.CurrentExp += ExpAmount;
	
	// 레벨업 체크
	while (CanLevelUp())
	{
		LevelData.CurrentExp -= LevelData.MaxExp;
		LevelData.CurrentLevel++;
		
		// 다음 레벨에 필요한 경험치 계산
		LevelData.MaxExp = LevelData.GetRequiredExpForLevel(LevelData.CurrentLevel + 1);
		
		// 레벨업 시 스탯 증가 처리
		ProcessLevelUpStatBonus();
		
		// 레벨업 이벤트 발생
		OnLevelUp.Broadcast(LevelData.CurrentLevel);
		
		UE_LOG(LogTemp, Log, TEXT("%s leveled up to %d!"), *GetOwner()->GetName(), LevelData.CurrentLevel);
	}
	
	// 경험치 변경 이벤트 발생
	if (LevelData.CurrentExp != OldExp)
	{
		OnExpChanged.Broadcast(LevelData.CurrentExp, LevelData.MaxExp);
	}
}

void UDiaStatComponent::SetLevel(int32 NewLevel)
{
	if (NewLevel <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid level value: %d"), NewLevel);
		return;
	}
	
	if (LevelData.CurrentLevel != NewLevel)
	{
		int32 OldLevel = LevelData.CurrentLevel;
		LevelData.CurrentLevel = NewLevel;
		
		// 레벨에 맞는 경험치 설정
		LevelData.MaxExp = LevelData.GetRequiredExpForLevel(NewLevel + 1);
		LevelData.CurrentExp = 0.f;
		
		// 레벨 변경 시 스탯 재계산
		RecalculateStatsForLevel(NewLevel);
		
		OnLevelUp.Broadcast(LevelData.CurrentLevel);
		
		UE_LOG(LogTemp, Log, TEXT("%s level changed from %d to %d"), 
			*GetOwner()->GetName(), OldLevel, NewLevel);
	}
}

bool UDiaStatComponent::CanLevelUp() const
{
	return LevelData.CurrentExp >= LevelData.MaxExp;
}

void UDiaStatComponent::ProcessLevelUpStatBonus()
{
	// 레벨업 시 스탯 증가
	float HealthIncrease = 10.0f * LevelData.CurrentLevel;
	float ManaIncrease = 5.0f * LevelData.CurrentLevel;
	float AttackIncrease = 2.0f * LevelData.CurrentLevel;
	
	SetMaxHealth(CharacterData.MaxHealth + HealthIncrease);
	SetMaxMana(CharacterData.MaxMana + ManaIncrease);
	SetAttackPower(CombatStats.AttackPower + AttackIncrease);
	
	// 체력과 마나를 완전 회복
	CharacterData.Health = CharacterData.MaxHealth;
	CharacterData.Mana = CharacterData.MaxMana;
	
	UE_LOG(LogTemp, Log, TEXT("%s gained stats: HP+%.1f, MP+%.1f, ATK+%.1f"), 
		*GetOwner()->GetName(), HealthIncrease, ManaIncrease, AttackIncrease);
}

void UDiaStatComponent::RecalculateStatsForLevel(int32 Level)
{
	// 레벨 변경 시 전체 스탯 재계산 (치트나 디버그용)
	float BaseHealth = 100.0f;
	float BaseMana = 50.0f;
	float BaseAttack = 10.0f;
	
	float NewMaxHealth = BaseHealth + (10.0f * (Level - 1));
	float NewMaxMana = BaseMana + (5.0f * (Level - 1));
	float NewAttackPower = BaseAttack + (2.0f * (Level - 1));
	
	SetMaxHealth(NewMaxHealth);
	SetMaxMana(NewMaxMana);
	SetAttackPower(NewAttackPower);
	
	// 체력과 마나를 완전 회복
	CharacterData.Health = CharacterData.MaxHealth;
	CharacterData.Mana = CharacterData.MaxMana;
} 