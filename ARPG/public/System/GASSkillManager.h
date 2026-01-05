#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/DiaGASSkillData.h"
#include "GASSkillManager.generated.h"

/**
 * GAS 스킬 관리자 - 스킬 데이터 관리 및 제공
 */
UCLASS()
class ARPG_API UGASSkillManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 스킬 데이터 가져오기
	UFUNCTION(BlueprintPure, Category = "GAS Skills")
	FGASSkillData GetSkillData(int32 SkillID) const;

	// 스킬 데이터 포인터 얻기(내부/성능용)
	const FGASSkillData* GetSkillDataPtr(int32 SkillID) const;

	// 모든 스킬 데이터 가져오기
	UFUNCTION(BlueprintCallable, Category = "GAS Skills")
	TArray<FGASSkillData> GetAllSkillData() const;

	// 스킬 타입별 필터링
	UFUNCTION(BlueprintCallable, Category = "GAS Skills")
	TArray<FGASSkillData> GetSkillsByType(EGASSkillType SkillType) const;

	const FSkillVariantData* GetSkllVariantDataPtr(int32 VariantID) const;
protected:
	// DataTable에서 스킬 데이터 로드
	void LoadSkillDataFromTable();
	void LoadSkillVariantDataFromTable();
public:
	// DataTable 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSoftObjectPtr<UDataTable> SkillDataTable;

	// DataTable 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSoftObjectPtr<UDataTable> SkillVariationTable;

	
protected:
	// 스킬 데이터 저장소
	UPROPERTY()
	TMap<int32, FGASSkillData> SkillDataMap;

	UPROPERTY()
	TMap<int32, FSkillVariantData> SkillVariantDataMap;
};