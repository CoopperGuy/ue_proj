// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Skill/DiaDamageType.h"
#include "DiaDamageCalculator.generated.h"

//struct FDamageResult
//{
//	float FinalDamage;
//	bool bIsCritical;
//	bool bIsBlocked;
//	bool bIsEvaded;
//	EDamageElementType ElementType;
//	TArray<UDiaStatusEffect*> AppliedStatusEffects;
//	
//	FDamageResult()
//		: FinalDamage(0.0f)
//		, bIsCritical(false)
//		, bIsBlocked(false)
//		, bIsEvaded(false)
//		, ElementType(EDamageElementType::EDT_MAX)
//	{}
//};

/**
 * 데미지 계산 및 적용을 담당하는 유틸리티 클래스
 */
UCLASS()
class ARPG_API UDiaDamageCalculator : public UObject
{
	GENERATED_BODY()
	
//public:
//	// 데미지 계산
//	static FDamageResult CalculateDamage(
//		float BaseDamage,
//		AActor* DamageDealer,
//		AActor* DamageReceiver,
//		const UDiaDamageType* DamageType,
//		float CriticalChance = 0.05f
//	);
//	
//	// 데미지 적용
//	static float ApplyDamage(
//		AActor* DamageDealer,
//		AActor* DamageReceiver,
//		float BaseDamage,
//		TSubclassOf<UDiaDamageType> DamageTypeClass,
//		float CriticalChance = 0.05f
//	);
//	
//	// 데미지 저항/약점 계산 (1.0이 기본, 0.5는 저항, 2.0은 약점)
//	static float CalculateElementalMultiplier(EDamageElementType AttackElement, EDamageElementType DefenderElement);
//	
//	// 넉백 적용
//	static void ApplyKnockback(AActor* Target, AActor* Source, float KnockbackForce);
//	
//private:
//	// 방어력 적용 데미지 계산
//	static float ApplyDefense(float Damage, float Defense, float ArmorPenetration);
//	
//	// 크리티컬 여부 확인
//	static bool RollForCritical(float CriticalChance);
//	
//	// 회피 여부 확인
//	static bool RollForEvasion(float EvasionChance);
//	
//	// 블록 여부 확인
//	static bool RollForBlock(float BlockChance);
//	
//	// 상태 이상 적용 여부 확인
//	static bool RollForStatusEffect(float StatusEffectChance);
};
