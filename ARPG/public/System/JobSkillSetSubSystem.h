// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/DiaGASSkillData.h"
#include "JobSkillSetSubSystem.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UJobSkillSetSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	const FJobSkillSet& GetJobSkillSet(EJobType JobType) const;
protected:
	void LoadJobSkillDataFromTable();

public:
	// DataTable 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSoftObjectPtr<UDataTable> JobSKillDataTable;

protected:
	// 스킬 데이터 저장소
	UPROPERTY()
	TMap<EJobType, FJobSkillSet> JobSkillSetMap;

	
};
