// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Types/DiaGASSkillData.h"
#include "DiaSkillLoadService.generated.h"

class USkillObject;
class UDiaSkillVariant;

/**
 * 스킬 데이터 로딩을 담당하는 Service
 */
UCLASS()
class ARPG_API UDiaSkillLoadService : public UObject
{
	GENERATED_BODY()
	
public:
	/**
	 * JobType에 해당하는 스킬 데이터를 로드하여 SkillObjects와 SkillVariants를 채웁니다.
	 * @param JobType 로드할 직업 타입
	 * @param OutSkillObjects 로드된 SkillObject들이 저장될 배열
	 * @param OutSkillVariants 로드된 SkillVariant들이 저장될 Map
	 * @param OuterObject SkillObject와 SkillVariant를 생성할 때 사용할 Outer 객체
	 */
	void LoadJobSkillData(
		EJobType JobType,
		TArray<USkillObject*>& OutSkillObjects,
		TMap<int32, UDiaSkillVariant*>& OutSkillVariants,
		UObject* OuterObject);
};
