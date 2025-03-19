// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "DiaDamageType.generated.h"

UENUM(BlueprintType)
enum class EDamageElementType : uint8
{
    Physical,
    Fire,
    Ice,
    Lightning,
    Poison,
    Holy,
    Shadow,
    None
};

class UDiaStatusEffect;
/**
 * 확장된 데미지 타입 클래스
 */
UCLASS()
class ARPG_API UDiaDamageType : public UDamageType
{
	GENERATED_BODY()
	
public:
	UDiaDamageType();
	
	// 데미지 속성
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	EDamageElementType ElementType;
	
	// 방어력 무시 비율 (0.0-1.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float ArmorPenetration;
	
	// 크리티컬 데미지 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float CriticalDamageMultiplier;
	
	// 넉백 강도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	float KnockbackForce;
	
	// 상태 이상 효과
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<TSubclassOf<UDiaStatusEffect>> StatusEffects;
	
	// 상태 이상 적용 확률 (0.0-1.0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	float StatusEffectChance;
};
