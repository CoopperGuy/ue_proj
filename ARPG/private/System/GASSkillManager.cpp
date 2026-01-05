#include "System/GASSkillManager.h"
#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "GAS/Abilities/DiaMeleeAbility.h"
#include "GAS/Abilities/DiaProjectileAbility.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"

void UGASSkillManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// DataTable에서 로드 시도, 실패 시 기본 데이터 사용
	LoadSkillDataFromTable();
	LoadSkillVariantDataFromTable();

	if (SkillDataMap.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GASSkillManager: DataTable load failed, using default data"));
	}
	
	UE_LOG(LogTemp, Log, TEXT("GASSkillManager: Initialized with %d skills"), SkillDataMap.Num());
}

const FGASSkillData* UGASSkillManager::GetSkillDataPtr(int32 SkillID) const
{
	if (const FGASSkillData* Found = SkillDataMap.Find(SkillID))
	{
		return Found;
	}
	return nullptr;
}

FGASSkillData UGASSkillManager::GetSkillData(int32 SkillID) const
{
	if (const FGASSkillData* Found = SkillDataMap.Find(SkillID))
	{
		return *Found;
	}
	return FGASSkillData{};
}

TArray<FGASSkillData> UGASSkillManager::GetAllSkillData() const
{
	TArray<FGASSkillData> AllSkills;
	for (const auto& Pair : SkillDataMap)
	{
		AllSkills.Add(Pair.Value);
	}
	return AllSkills;
}

TArray<FGASSkillData> UGASSkillManager::GetSkillsByType(EGASSkillType SkillType) const
{
	TArray<FGASSkillData> FilteredSkills;
	for (const auto& Pair : SkillDataMap)
	{
		if (Pair.Value.SkillType == SkillType)
		{
			FilteredSkills.Add(Pair.Value);
		}
	}
	return FilteredSkills;
}

const FSkillVariantData* UGASSkillManager::GetSkllVariantDataPtr(int32 VariantID) const
{
	if (const FSkillVariantData* Found = SkillVariantDataMap.Find(VariantID))
	{
		return Found;
	}
	return nullptr;
}

void UGASSkillManager::LoadSkillDataFromTable()
{
	// DataTable 경로 설정 (프로젝트 설정에서 변경 가능)
	if (!SkillDataTable.IsValid())
	{
		// 기본 DataTable 경로 설정
		SkillDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Datatable/DT_GASSkillData.DT_GASSkillData")));
	}

	UDataTable* DataTable = SkillDataTable.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GASSkillManager: Failed to load DataTable at path: %s"), 
			*SkillDataTable.ToString());
		return;
	}

	// DataTable 구조체 검증
	if (DataTable->GetRowStruct() != FGASSkillData::StaticStruct())
	{
		UE_LOG(LogTemp, Error, TEXT("GASSkillManager: DataTable row struct mismatch!"));
		return;
	}

	// 모든 행 가져오기
	TArray<FName> RowNames = DataTable->GetRowNames();
	SkillDataMap.Empty();

	for (const FName& RowName : RowNames)
	{
		FGASSkillData* RowData = DataTable->FindRow<FGASSkillData>(RowName, TEXT("GASSkillManager"));
		if (RowData)
		{
			SkillDataMap.Add(RowData->SkillID, *RowData);
			UE_LOG(LogTemp, Log, TEXT("GASSkillManager: Loaded skill %d: %s"), 
				RowData->SkillID, *RowData->SkillName.ToString());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GASSkillManager: Loaded %d skills from DataTable"), SkillDataMap.Num());
}

void UGASSkillManager::LoadSkillVariantDataFromTable()
{
	// DataTable 경로 설정 (프로젝트 설정에서 변경 가능)
	if (!SkillVariationTable.IsValid())
	{
		// 기본 DataTable 경로 설정
		SkillVariationTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Datatable/DT_SkillVariantion.DT_SkillVariantion")));
	}

	UDataTable* DataTable = SkillVariationTable.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GASSkillManager: Failed to load DataTable at path: %s"),
			*SkillVariationTable.ToString());
		return;
	}

	// DataTable 구조체 검증
	if (DataTable->GetRowStruct() != FSkillVariantData::StaticStruct())
	{
		UE_LOG(LogTemp, Error, TEXT("GASSkillManager: DataTable row struct mismatch!"));
		return;
	}

	// 모든 행 가져오기
	TArray<FName> RowNames = DataTable->GetRowNames();
	SkillVariantDataMap.Empty();

	for (const FName& RowName : RowNames)
	{
		FSkillVariantData* RowData = DataTable->FindRow<FSkillVariantData>(RowName, TEXT(""));
		if (RowData)
		{
			SkillVariantDataMap.Add(RowData->VariantID, *RowData);
			UE_LOG(LogTemp, Log, TEXT("GASSkillManager: Loaded skill %d"),
				RowData->VariantID);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GASSkillManager: Loaded %d skills from DataTable"), SkillVariantDataMap.Num());
}
