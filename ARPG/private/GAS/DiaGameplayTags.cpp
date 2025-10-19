// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/DiaGameplayTags.h"
#include "GameplayTagsManager.h"

FDiaGameplayTags FDiaGameplayTags::GameplayTags;

void FDiaGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// ========================================
	// State Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.State_CastingSkill, "GASData.State.CastingSkill", "스킬 시전 중 - 이동 불가");
	GameplayTags.AddTag(GameplayTags.State_Charging, "GASData.State.Charging", "돌진 준비 중 - 이동 불가");
	GameplayTags.AddTag(GameplayTags.State_Stunned, "GASData.State.Stunned", "스턴 상태");
	GameplayTags.AddTag(GameplayTags.State_Invincible, "GASData.State.Invincible", "무적 상태");

	// ========================================
	// Cooldown Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.CoolDown_1001, "GASData.CoolDown.1001", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_1002, "GASData.CoolDown.1002", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2001, "GASData.CoolDown.2001", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2002, "GASData.CoolDown.2002", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2003, "GASData.CoolDown.2003", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2004, "GASData.CoolDown.2004", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2005, "GASData.CoolDown.2005", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2006, "GASData.CoolDown.2006", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2007, "GASData.CoolDown.2007", "");
	GameplayTags.AddTag(GameplayTags.CoolDown_2008, "GASData.CoolDown.2008", "");

	// ========================================
	// Skill Type Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.SkillType_Character_Player, "SkillType.Character.Player", "");
	GameplayTags.AddTag(GameplayTags.SkillType_Character_Monster, "SkillType.Character.Monster", "");
	GameplayTags.AddTag(GameplayTags.SkillType_Character_BossMonster, "SkillType.Character.BossMonster", "");
	GameplayTags.AddTag(GameplayTags.SkillType_Character_BossMonster_DualBlade, "SkillType.Character.BossMonster.DualBlade", "");

	// ========================================
	// Ability Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.Ability_Cost_Mana_NotEnough, "Ability.Cost.Mana.NotEnough", "");

	// ========================================
	// Damage Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.Damage_Base, "GASData.DamageBase", "");
	GameplayTags.AddTag(GameplayTags.Damage_Crit, "GASData.CritMultiplier", "");
	GameplayTags.AddTag(GameplayTags.Damage_Final, "GASData.Damage", "");

	// ========================================
	// UI Tags
	// ========================================
	GameplayTags.AddTag(GameplayTags.UI_Layer_HUD, "UI.Layer.HUD", "");
	GameplayTags.AddTag(GameplayTags.UI_Layer_Modal, "UI.Layer.Model", "");
}

void FDiaGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName), FString(TEXT("(Native) ")) + FString(TagComment));
}

