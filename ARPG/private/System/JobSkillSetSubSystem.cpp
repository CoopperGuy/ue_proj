// Fill out your copyright notice in the Description page of Project Settings.


#include "System/JobSkillSetSubSystem.h"

void UJobSkillSetSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// DataTable에서 로드 시도, 실패 시 기본 데이터 사용
	LoadJobSkillDataFromTable();

	if (JobSkillSetMap.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UJobSkillSetSubSystem: DataTable load failed, using default data"));
	}

	UE_LOG(LogTemp, Log, TEXT("UJobSkillSetSubSystem: Initialized with %d skills"), JobSkillSetMap.Num());

}

const FJobSkillSet& UJobSkillSetSubSystem::GetJobSkillSet(EJobType JobType) const
{
	if (const FJobSkillSet* FoundSet = JobSkillSetMap.Find(JobType))
	{
		return *FoundSet;
	}
	static FJobSkillSet EmptySet;
	return EmptySet;
}

void UJobSkillSetSubSystem::LoadJobSkillDataFromTable()
{
	if (!JobSKillDataTable.IsValid())
	{
		JobSKillDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Datatable/DT_JobSkillSet.DT_JobSkillSet")));

		JobSKillDataTable.LoadSynchronous();
	}
	if (UDataTable* DataTable = JobSKillDataTable.Get())
	{
		JobSkillSetMap.Empty();
		TArray<FName> RowNames = DataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			FJobSkillSet* JobSkillSetRow = DataTable->FindRow<FJobSkillSet>(RowName, TEXT(""));
			if (JobSkillSetRow)
			{
				JobSkillSetMap.Emplace(JobSkillSetRow->JobType, *JobSkillSetRow);
			}
		}
	}
}
