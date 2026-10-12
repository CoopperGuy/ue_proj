// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "NativeGameplayTags.h"

namespace DiaGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_Level);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_MaxHealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_Mana);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_MaxMana);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_Exp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_MaxExp);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_Strength);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_Dexterity);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_Intelligence);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_AttackPower);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_Defense);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_DamageIncreaseOption);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_MovementSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_IncomingDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_IncomingHealing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_CritChance);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_CritDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(AttributeSet_CooldownReduction);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_LifeSteal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_CooldownReduction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_GoldFind);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_MaxHealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_MovementSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_CritChance);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_DamageIncreaseOption);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_AttackPower);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_Defense);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_MaxMana);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_CritDamage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_HealthOnKill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_ManaOnKill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_Execution);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_Ambush);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_Berserker);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_Guardian);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_Frostbound);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ItemOption_Lifeline);

}

/**
 * DiaGameplayTags
 * 
 * 프로젝트 전체에서 사용하는 GameplayTag들을 중앙 관리하는 싱글톤 클래스
 * 
 * 사용법:
 *   FDiaGameplayTags::Get().State_Charging
 *   FDiaGameplayTags::Get().CoolDown_GetTag(SkillID)
 */
struct FDiaGameplayTags
{
public:
	static const FDiaGameplayTags& Get() { return GameplayTags; }
	static void InitializeNativeTags();

private:
	static void RegisterStateTags();
	static void RegisterCoolDownTags();
	static void RegisterSkillTypeTags();
	static void RegisterAbilityTags();
	static void RegisterDamageTags();
	static void RegisterBuildTags();
	static void RegisterUITags();
	static void RegisterAttributeSetTags();
	static void RegisterItemOptionTags();
	static void RegisterCooldownTagsFromDataTable();
public:
	FGameplayTag Actor_Player;	// 플레이어
	FGameplayTag Actor_Monster;	// 몬스터

	FGameplayTag SkillActor_Ground; // SkillActor 중 그라운드 오브젝트 태그 (예: 스킬의 지속 효과로 바닥에 남는 오브젝트)
	FGameplayTag SkillActor_Projectile; // SkillActor 중 탄환 오브젝트 태그 
	FGameplayTag SkillActor_Default; // SkillActor 중 기본 오브젝트 태그 

	// ========================================
	// State Tags - 캐릭터 상태
	// ========================================
	FGameplayTag State_CastingSkill;	// 스킬 시전 중 - 이동 불가
	FGameplayTag State_Charging;		// 돌진 준비 중 - 이동 불가
	FGameplayTag State_Stunned;			// 스턴 상태 (향후 확장용)
	FGameplayTag State_Invincible;		// 무적 상태 (향후 확장용)
	FGameplayTag State_Slowed;			// 느려짐 상태 (향후 확장용)
	FGameplayTag State_Freeze;			// 프리즈 상태 
	FGameplayTag State_KnockBack; 		// 넉백 상태
	// ========================================
	// Cooldown Tags - 쿨다운
	// ========================================
	FGameplayTag CoolDown_1001;
	FGameplayTag CoolDown_1002;
	FGameplayTag CoolDown_1003;
	FGameplayTag CoolDown_1004;
	FGameplayTag CoolDown_1005;
	FGameplayTag CoolDown_1006;
	FGameplayTag CoolDown_1007;
	FGameplayTag CoolDown_1008;
	FGameplayTag CoolDown_1009;
	FGameplayTag CoolDown_1010;
	FGameplayTag CoolDown_1011;
	FGameplayTag CoolDown_2001;
	FGameplayTag CoolDown_2002;
	FGameplayTag CoolDown_2003;
	FGameplayTag CoolDown_2004;
	FGameplayTag CoolDown_2005;
	FGameplayTag CoolDown_2006;
	FGameplayTag CoolDown_2007;
	FGameplayTag CoolDown_2008;

	// ========================================
	// Skill Type Tags - 스킬 분류
	// ========================================
	FGameplayTag SkillType_Character_Player;
	FGameplayTag SkillType_Character_Monster;
	FGameplayTag SkillType_Character_BossMonster;
	FGameplayTag SkillType_Character_BossMonster_DualBlade;

	// ========================================
	// Ability Tags - 어빌리티 관련
	// ========================================
	FGameplayTag Ability_Cost_Mana_NotEnough;
	FGameplayTag GASData_Exp;
	FGameplayTag GASData_Duration;
	FGameplayTag GASData_StatusValue;
	FGameplayTag GASData_MultipleShot;
	FGameplayTag GASData_Pierce;
	FGameplayTag GASData_Variant_DamageMultiplier;
	FGameplayTag GASData_Variant_CooldownReduction;
	FGameplayTag GASData_Variant_ManaCostMultiplier;
	FGameplayTag GASData_Variant_SlowOnHit;
	FGameplayTag GASData_Variant_ExplodeOnHit;
	FGameplayTag GASData_Variant_Chain;
	FGameplayTag GASData_Variant_Fork;
	FGameplayTag GASData_Variant_AreaMultiplier;
	FGameplayTag GASData_Variant_DurationMultiplier;
	FGameplayTag GASData_Variant_RepeatCast;
	FGameplayTag GASData_Variant_FreezeStack;
	FGameplayTag GASData_Variant_BurningGround;
	FGameplayTag GASData_Variant_LifeSteal;
	FGameplayTag GASData_Variant_Knockback;
	FGameplayTag GASData_Variant_BigProjectile;
	FGameplayTag GASData_Variant_CloseRangeBonus;
	FGameplayTag GASData_Variant_PullToCenter;
	FGameplayTag GASData_Variant_Echo;


	// ========================================
	// Damage Tags - 데미지 관련
	// ========================================
	FGameplayTag Damage_Base;
	FGameplayTag Damage_Crit;
	FGameplayTag Damage_Final;
	FGameplayTag Damage_Multiplier;

	// ========================================
	// Build Tags - 보상 빌드 분류
	// ========================================
	FGameplayTag Build_Root;
	FGameplayTag Build_Mechanic_Projectile;
	FGameplayTag Build_Mechanic_Melee;
	FGameplayTag Build_Mechanic_Area;
	FGameplayTag Build_Mechanic_MultiHit;

	FGameplayTag Build_Damage_Physical;
	FGameplayTag Build_Damage_Fire;
	FGameplayTag Build_Damage_Cold;

	FGameplayTag Build_Stat_Crit;
	FGameplayTag Build_Stat_Cooldown;
	FGameplayTag Build_Stat_MaxHealth;

	FGameplayTag Build_Role_Damage;
	FGameplayTag Build_Role_Defense;
	FGameplayTag Build_Role_Mobility;
	FGameplayTag Build_Role_Control;
	FGameplayTag Build_Role_Buff;

	FGameplayTag Build_Modifier_Pierce;

	// ========================================
	// UI Tags
	// ========================================
	FGameplayTag UI_Layer_HUD;
	FGameplayTag UI_Layer_Modal;
	FGameplayTag UI_Layer_Menu;
	FGameplayTag UI_MenuPage_Skill;
	FGameplayTag UI_MenuPage_Inventory;
	FGameplayTag UI_MenuPage_Character;
	FGameplayTag UI_MenuPage_Settings;


	// ========================================
	// AttributeSet Tags - 기본 스탯
	// ========================================
	FGameplayTag AttributeSet_Level;			// Level
	FGameplayTag AttributeSet_Health;			// 체력
	FGameplayTag AttributeSet_MaxHealth;		// 최대 체력
	FGameplayTag AttributeSet_Mana;				// 마나
	FGameplayTag AttributeSet_MaxMana;			// 최대 마나
	FGameplayTag AttributeSet_Exp;				// 경험치
	FGameplayTag AttributeSet_MaxExp;			// 최대 경험치
	FGameplayTag AttributeSet_Strength;			// 힘 - 기본 스탯
	FGameplayTag AttributeSet_Dexterity;		// 민첩 - 기본 스탯
	FGameplayTag AttributeSet_Intelligence;		// 지능 - 기본 스탯
	FGameplayTag AttributeSet_AttackPower;		// 공격력 - 전투 스탯
	FGameplayTag AttributeSet_Defense;			// 방어력 - 전투 스탯
	FGameplayTag AttributeSet_DamageIncreaseOption;
	FGameplayTag AttributeSet_MovementSpeed;		// 이동 속도
	FGameplayTag AttributeSet_IncomingDamage;	// 받는 피해 - 메타 스탯
	FGameplayTag AttributeSet_IncomingHealing;	// 받는 회복 - 메타 스탯
	FGameplayTag AttributeSet_CritChance;		// 치명타 확률 - 전투 스탯
	FGameplayTag AttributeSet_CritDamage;		// 치명타 피해 - 전투 스탯
	FGameplayTag AttributeSet_CooldownReduction;	// 쿨다운 감소 - 전투 스탯

	FGameplayTag ItemOptionLifeSteal;			// 생명력 흡수
	FGameplayTag ItemOptionCooldownReduction;	// 쿨다운 감소
	FGameplayTag ItemOptionGoldFind;			// 골드 획득량 증가
	FGameplayTag ItemOptionStatDamage;			// 공격력 증가
	FGameplayTag ItemOptionstatHealth;			// 체력 증가
	FGameplayTag ItemOptionStatSpeed;			// 이동 속도 증가
	FGameplayTag ItemOptionStatCriticalChance;  // 치명타 확률 증가

	/**
	 * 스킬 ID로 쿨다운 태그 가져오기
	 * @param SkillID 스킬 ID (예: 2003)
	 * @return GASData.CoolDown.2003
	 */
	static FGameplayTag CoolDown_GetTag(int32 SkillID)
	{
		FName SkillIDTagName = FName(*FString::Printf(TEXT("GASData.CoolDown.%d"), SkillID));
		FString TagComment = FString::Printf(TEXT("스킬 ID %d의 쿨다운 태그"), SkillID);
		FGameplayTag FindVal = FGameplayTag::RequestGameplayTag(SkillIDTagName, false);
		if(!FindVal.IsValid())
		{
			FindVal = UGameplayTagsManager::Get().AddNativeGameplayTag(SkillIDTagName, TagComment);
			return FindVal;
		}
		return FindVal;
	}
	static const TArray<FGameplayTag>& GetAttributeStats();
	static const TArray<FGameplayTag>& GetItemOptionList();
protected:
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment); 
private:
	static FDiaGameplayTags GameplayTags;
};

