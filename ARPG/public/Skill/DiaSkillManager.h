// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Skill/DiaSkillDataTable.h"
#include "Skill/DiaSkillType.h"
#include "DiaSkillManager.generated.h"

class ADiaSkillBase;

/**
 * 스킬 시스템의 전반적인 관리를 담당하는 매니저 클래스
 */
UCLASS()
class ARPG_API UDiaSkillManager : public UObject
{
	GENERATED_BODY()

public:
	UDiaSkillManager();
	
	void Initialize();

	// 스킬 등록
	//Map 구조랑 동일한 순서
	bool RegisterSkillData(int32 SkillID, const FSkillData& SkillData);

	// 스킬 해제
	void UnregisterSkillData(int32 SkillID);

	// 포인터로 반환하는 const 함수
	const FSkillData* GetSkillData(int32 SkillID) const;
	const FSkillData* GetSkillData(FString SkillName) const;

	// 복사가 필요한 경우를 위한 함수
	bool CopySkillData(FSkillData& OutSkillData, int32 SkillID) const;

	const TMap<int32, FSkillData>& GetSkillDataMap() const { return SkillDataMap; }
protected:
    UPROPERTY()
    UDataTable* SkillDataTable;

    // 데이터 테이블 경로
    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    FString SkillDataTablePath = TEXT("/Game/Blueprint/Datatable/Skill/DT_SkillDataRow");
private:
	// 스킬 데이터 맵 (스킬 ID -> 스킬 데이터)
	UPROPERTY()
	TMap<int32, FSkillData> SkillDataMap;

	// 기본 스킬 데이터 로드
	void LoadDefaultSkillData();
};
