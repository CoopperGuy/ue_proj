// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DiaSkillActivationService.generated.h"

class ADiaBaseCharacter;
class USkillObject;

/**
 * 스킬 활성화를 담당하는 Service
 */
UCLASS()
class ARPG_API UDiaSkillActivationService : public UObject
{
	GENERATED_BODY()
	
public:
	/**
	 * 스킬 ID를 기반으로 스킬을 활성화합니다.
	 * @param SkillID 활성화할 스킬 ID
	 * @param Character 스킬을 사용할 캐릭터
	 * @param SkillObj 스킬 정보를 담고 있는 SkillObject (nullptr 가능)
	 * @return 활성화 성공 여부
	 */
	bool TryActivateSkill(int32 SkillID, ADiaBaseCharacter* Character, const USkillObject* SkillObj);
};
