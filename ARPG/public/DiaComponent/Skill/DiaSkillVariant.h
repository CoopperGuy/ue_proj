// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "DiaSkillVariant.generated.h"


class ADiaSkillActor;
/// <summary>
/// Spec은 Variant의 구체적인 수치 데이터를 담고있고,
/// Context는 Variant가 적용되는 상황에 대한 정보를 담고있다.
/// 건널목 같은것 (예를 들어 piece **관통** 이라는 옵션이 있을때, 이 옵션이 관통한 횟수를 카운팅하고 저장해야하니까
/// 또한 상황에 따른 함수도 실행해야한다.
/// 처음에는 ability나, skillobject이런곳에 하나하나 상황에 대하여 구현하려고 했는데 너무 방대해 지고 가독성 유지보수성이 떨어질거같았음.
/// 
/// </summary>

USTRUCT(BlueprintType)
struct ARPG_API FDiaSkillVariantSpec
{
	GENERATED_BODY()

	FDiaSkillVariantSpec()
	{ }

	float ModifierValue;
	FGameplayTag SkillTag;
};


USTRUCT(BlueprintType)
struct ARPG_API FDiaSkillVariantContext
{
	GENERATED_BODY()
	
	FDiaSkillVariantContext() {}
	FDiaSkillVariantContext(const FDiaSkillVariantSpec& spec) {}

	FHitResult HitResult;
	TSubclassOf<class ADiaSkillActor> SkillActorClass;
	FGameplayAbilityTargetDataHandle TargetData;
	class UAbilitySystemComponent* AbilityComp;
	class ADiaSkillActor* SkillActor;
};

/**
 * 
 */
UCLASS()
class ARPG_API UDiaSkillVariant : public UObject
{
	GENERATED_BODY()
	
public:
	UDiaSkillVariant();

	void InitializeVariant(const FDiaSkillVariantSpec& spec);

	const FDiaSkillVariantSpec GetVariantSpec() const { return VariantSpec; }

protected:
	FDiaSkillVariantSpec VariantSpec;

	int32 SkillID;
	FText SkillVariantName;
	FText SkillVariantDescription;

public:
	void SetSkillID(int32 InSkillID) { SkillID = InSkillID; }
	void SetSkillVariantName(const FText& InName) { SkillVariantName = InName; }
	void SetSkillVariantDescription(const FText& InDescription) { SkillVariantDescription = InDescription; }

	int32 GetSkillID() const { return SkillID; }
	FText GetSkillVariantName() const { return SkillVariantName; }
	FText GetSkillVariantDescription() const { return SkillVariantDescription; }
};
