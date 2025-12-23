// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/DiaGroundAbility.h"

void UDiaGroundAbility::InitializeWithSkillData(const FGASSkillData& InSkillData)
{
    Super::InitializeWithSkillData(InSkillData);
    //if (InSkillData.ProjectileClass)
    //{
    //    ProjectileClass = InSkillData.ProjectileClass;
    //}
    //// Multi-shot 같은 값은 데이터 구조에 따라 매핑 (예시로 Count/Spread를 Range/Radius로 매핑)
    //if (InSkillData.SkillType == EGASSkillType::RangedAttack || InSkillData.SkillType == EGASSkillType::Magic)
    //{
    //    // 선택적 매핑 규칙: Range -> ProjectileCount(정수 변환), Radius -> SpreadAngle
    //    ProjectileCount = FMath::Clamp(static_cast<int32>(InSkillData.Range / 200.0f), 1, 7);
    //    bFireMultipleProjectiles = ProjectileCount > 1;
    //    SpreadAngle = FMath::Clamp(InSkillData.Radius * 0.2f, 5.0f, 45.0f);
    //}

}

void UDiaGroundAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SpawnSkillGround();
}

void UDiaGroundAbility::SpawnSkillGround()
{
}
