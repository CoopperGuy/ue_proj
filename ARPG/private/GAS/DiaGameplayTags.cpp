// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/DiaGameplayTags.h"
#include "GameplayTagsManager.h"

FDiaGameplayTags FDiaGameplayTags::GameplayTags;

void FDiaGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	RegisterStateTags();
	RegisterCoolDownTags();
	RegisterSkillTypeTags();
	RegisterAbilityTags();
	RegisterDamageTags();
	RegisterUITags();
	RegisterAttributeSetTags();
	RegisterItemOptionTags();
}

void FDiaGameplayTags::RegisterStateTags()
{	// ========================================
	// State Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.State_CastingSkill, "GASData.State.CastingSkill", "스킬 시전 중 - 이동 불가");
	GameplayTags.AddTag(GameplayTags.State_Charging, "GASData.State.Charging", "돌진 준비 중 - 이동 불가");
	GameplayTags.AddTag(GameplayTags.State_Stunned, "GASData.State.Stunned", "스턴 상태");
	GameplayTags.AddTag(GameplayTags.State_Invincible, "GASData.State.Invincible", "무적 상태");

}

void FDiaGameplayTags::RegisterCoolDownTags()
{	// ========================================
	// Cooldown Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.CoolDown_1001, "GASData.CoolDown.1001", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1002, "GASData.CoolDown.1002", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1003, "GASData.CoolDown.1003", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1004, "GASData.CoolDown.1004", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1005, "GASData.CoolDown.1005", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1006, "GASData.CoolDown.1006", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1007, "GASData.CoolDown.1007", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1008, "GASData.CoolDown.1008", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1009, "GASData.CoolDown.1009", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1010, "GASData.CoolDown.1010", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1011, "GASData.CoolDown.1011", "");

	GameplayTags.AddTag(GameplayTags.CoolDown_2001, "GASData.CoolDown.2001", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2002, "GASData.CoolDown.2002", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2003, "GASData.CoolDown.2003", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2004, "GASData.CoolDown.2004", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2005, "GASData.CoolDown.2005", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2006, "GASData.CoolDown.2006", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2007, "GASData.CoolDown.2007", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2008, "GASData.CoolDown.2008", "");

}

void FDiaGameplayTags::RegisterSkillTypeTags()
{	// ========================================
	// Skill Type Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.SkillType_Character_Player, "SkillType.Character.Player", "");
	GameplayTags.AddTag(GameplayTags.SkillType_Character_Monster, "SkillType.Character.Monster", "");
	GameplayTags.AddTag(GameplayTags.SkillType_Character_BossMonster, "SkillType.Character.BossMonster", "");
	GameplayTags.AddTag(GameplayTags.SkillType_Character_BossMonster_DualBlade, "SkillType.Character.BossMonster.DualBlade", "");

}

void FDiaGameplayTags::RegisterAbilityTags()
{	// ========================================
	// Ability Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.Ability_Cost_Mana_NotEnough, "Ability.Cost.Mana.NotEnough", "");
	GameplayTags.AddTag(GameplayTags.GASData_Exp, "GASData.Exp", "");

}

void FDiaGameplayTags::RegisterDamageTags()
{	// ========================================
	// Damage Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.Damage_Base, "GASData.DamageBase", "");
	GameplayTags.AddTag(GameplayTags.Damage_Crit, "GASData.CritMultiplier", "");
	GameplayTags.AddTag(GameplayTags.Damage_Final, "GASData.Damage", "");

}

void FDiaGameplayTags::RegisterUITags()
{	// ========================================
	// UI Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.UI_Layer_HUD, "UI.Layer.HUD", "");
	GameplayTags.AddTag(GameplayTags.UI_Layer_Modal, "UI.Layer.Model", "");

}

void FDiaGameplayTags::RegisterAttributeSetTags()
{	// ========================================
	// AttributeSet Tags - 기본 스탯
	// ========================================
	GameplayTags.AddTag(GameplayTags.AttributeSet_Health, "AttributeSet.Health", "체력");
	GameplayTags.AddTag(GameplayTags.AttributeSet_MaxHealth, "AttributeSet.MaxHealth", "최대 체력");
	GameplayTags.AddTag(GameplayTags.AttributeSet_Mana, "AttributeSet.Mana", "마나");
	GameplayTags.AddTag(GameplayTags.AttributeSet_MaxMana, "AttributeSet.MaxMana", "최대 마나");
	GameplayTags.AddTag(GameplayTags.AttributeSet_Exp, "AttributeSet.Exp", "경험치");
	GameplayTags.AddTag(GameplayTags.AttributeSet_MaxExp, "AttributeSet.MaxExp", "최대 경험치");
	GameplayTags.AddTag(GameplayTags.AttributeSet_Strength, "AttributeSet.Strength", "힘 - 기본 스탯");
	GameplayTags.AddTag(GameplayTags.AttributeSet_Dexterity, "AttributeSet.Dexterity", "민첩 - 기본 스탯");
	GameplayTags.AddTag(GameplayTags.AttributeSet_Intelligence, "AttributeSet.Intelligence", "지능 - 기본 스탯");
	GameplayTags.AddTag(GameplayTags.AttributeSet_AttackPower, "AttributeSet.AttackPower", "공격력 - 전투 스탯");
	GameplayTags.AddTag(GameplayTags.AttributeSet_Defense, "AttributeSet.Defense", "방어력 - 전투 스탯");
	GameplayTags.AddTag(GameplayTags.AttributeSet_MovementSpeed, "AttributeSet.MovementSpeed", "이동 속도");
}

void FDiaGameplayTags::RegisterItemOptionTags()
{
	// ========================================
	// Item Option Tags - 아이템 옵션
	// ========================================
	
	GameplayTags.AddTag(GameplayTags.ItemOptionLifeSteal, "Item.Option.MultiplyAdditive.LifeSteal", "생명력 흡수");
	GameplayTags.AddTag(GameplayTags.ItemOptionCooldownReduction, "Item.Option.MultiplyAdditive.CooldownReduction", "쿨다운 감소");
	GameplayTags.AddTag(GameplayTags.ItemOptionGoldFind, "Item.Option.MultiplyAdditive.GoldFind", "골드 획득량 증가");
	GameplayTags.AddTag(GameplayTags.ItemOptionStatDamage, "Item.Option.Additive.Health", "공격력 증가");
	GameplayTags.AddTag(GameplayTags.ItemOptionstatHealth, "Item.Option.MultiplyAdditive.Speed", "체력 증가");
	GameplayTags.AddTag(GameplayTags.ItemOptionStatSpeed, "Item.Option.Additive.CriticalChance", "이동 속도 증가");
	GameplayTags.AddTag(GameplayTags.ItemOptionStatCriticalChance, "Item.Option.MultiplyAdditive.DamageIncreaseOption", "치명타 확률 증가");
}

const TArray<FGameplayTag>& FDiaGameplayTags::GetAttributeStats()
{
	static TArray<FGameplayTag> CacheStats;

	if(CacheStats.Num() > 0)
	{
		return CacheStats;
	}

	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Health")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.MaxHealth")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Mana")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.MaxMana")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Exp")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.MaxExp")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Strength")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Dexterity")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Intelligence")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.AttackPower")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.Defense")));
	CacheStats.Add(FGameplayTag::RequestGameplayTag(FName("AttributeSet.MovementSpeed")));


	return CacheStats;
}

const TArray<FGameplayTag>& FDiaGameplayTags::GetItemOptionList()
{
	static TArray<FGameplayTag> CacheOption;

	if(CacheOption.Num() > 0)
	{
		return CacheOption;
	}

	CacheOption.Add(FGameplayTag::RequestGameplayTag(FName("Item.Option.MultiplyAdditive.LifeSteal")));
	CacheOption.Add(FGameplayTag::RequestGameplayTag(FName("Item.Option.MultiplyAdditive.CooldownReduction")));
	CacheOption.Add(FGameplayTag::RequestGameplayTag(FName("Item.Option.MultiplyAdditive.GoldFind")));
	CacheOption.Add(FGameplayTag::RequestGameplayTag(FName("Item.Option.Additive.Health")));
	CacheOption.Add(FGameplayTag::RequestGameplayTag(FName("Item.Option.MultiplyAdditive.Speed")));
	CacheOption.Add(FGameplayTag::RequestGameplayTag(FName("Item.Option.Additive.CriticalChance")));
	CacheOption.Add(FGameplayTag::RequestGameplayTag(FName("Item.Option.MultiplyAdditive.DamageIncreaseOption")));

	return CacheOption;
}

void FDiaGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName), FString(TEXT("(Native) ")) + FString(TagComment));
}
