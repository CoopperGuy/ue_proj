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

	// ========================================
	// State Tags - 캐릭터 상태
	// ========================================
	FGameplayTag State_CastingSkill;	// 스킬 시전 중 - 이동 불가
	FGameplayTag State_Charging;		// 돌진 준비 중 - 이동 불가
	FGameplayTag State_Stunned;			// 스턴 상태 (향후 확장용)
	FGameplayTag State_Invincible;		// 무적 상태 (향후 확장용)

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
	// Helper Functions - 동적 태그 생성
	// ========================================
	
	/**
	 * 스킬 ID로 쿨다운 태그 가져오기
	 * @param SkillID 스킬 ID (예: 2003)
	 * @return GASData.CoolDown.2003
	 */
	static FGameplayTag CoolDown_GetTag(int32 SkillID)
	{
		return FGameplayTag::RequestGameplayTag(FName(*FString::Printf(TEXT("GASData.CoolDown.%d"), SkillID)));
	}

protected:
	// 태그 등록 헬퍼 함수
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);

private:
	static FDiaGameplayTags GameplayTags;
};

