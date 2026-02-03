// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "DiaSkillVariantExecutorService.generated.h"

class UDiaGameplayAbility;
class UDiaSkillVariant;
class UDiaSkillVariantEffect;
/**
 * Variant 실행을 담당하는 Service
 */
UCLASS()
class ARPG_API UDiaSkillVariantExecutorService : public UObject
{
	GENERATED_BODY()
	
public:
	void InitializeExecutorService();

	/**
	 * Variant ID 배열을 기반으로 Variant들을 실행합니다.
	 * @param VariantIDs 실행할 Variant ID 배열
	 * @param VariantCache Variant 캐시 (VariantID -> UDiaSkillVariant*)
	 * @param Context Variant 실행에 필요한 컨텍스트
	 * @param Ability 스킬 어빌리티
	 */
	void ExecuteVariants(
		const TArray<int32>& VariantIDs,
		const TMap<int32, UDiaSkillVariant*>& VariantCache,
		FDiaSkillVariantContext& Context,
		UDiaGameplayAbility* Ability);

protected:
};
