// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

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
	static void RegisterUITags();
	static void RegisterAttributeSetTags();
	static void RegisterItemOptionTags();
public:
	// ========================================
	// State Tags - 캐릭터 상태
	// ========================================
	FGameplayTag State_CastingSkill;	// 스킬 시전 중 - 이동 불가
	FGameplayTag State_Charging;		// 돌진 준비 중 - 이동 불가
	FGameplayTag State_Stunned;			// 스턴 상태 (향후 확장용)
	FGameplayTag State_Invincible;		// 무적 상태 (향후 확장용)
	FGameplayTag State_Slowed;			// 느려짐 상태 (향후 확장용)
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
	FGameplayTag GASData_MultipleShot;


	// ========================================
	// Damage Tags - 데미지 관련
	// ========================================
	FGameplayTag Damage_Base;
	FGameplayTag Damage_Crit;
	FGameplayTag Damage_Final;

	// ========================================
	// UI Tags
	// ========================================
	FGameplayTag UI_Layer_HUD;
	FGameplayTag UI_Layer_Modal;

	// ========================================
	// AttributeSet Tags - 기본 스탯
	// ========================================
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
	FGameplayTag AttributeSet_MovementSpeed;		// 이동 속도
	FGameplayTag AttributeSet_IncomingDamage;	// 받는 피해 - 메타 스탯
	FGameplayTag AttributeSet_IncomingHealing;	// 받는 회복 - 메타 스탯
	
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
		return FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("GASData.CoolDown.%d"), SkillID)));
	}
	static const TArray<FGameplayTag>& GetAttributeStats();
	static const TArray<FGameplayTag>& GetItemOptionList();
protected:
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment); 
private:
	static FDiaGameplayTags GameplayTags;
};

