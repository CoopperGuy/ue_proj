// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/DiaSkillManager.h"
#include "Skill/DiaSkillBase.h"
#include "Skill/Active/DiaProjectileSkill.h"

UDiaSkillManager::UDiaSkillManager()
{
}


void UDiaSkillManager::Initialize()
{
    LoadDefaultSkillData();
}

const FSkillData* UDiaSkillManager::GetSkillData(int32 SkillID) const
{
    return SkillDataMap.Find(SkillID);
}

const FSkillData* UDiaSkillManager::GetSkillData(FString SkillName) const
{
    // SkillDataMap을 순회하면서 SkillName이 일치하는 데이터를 찾습니다
    for (const auto& Pair : SkillDataMap)
    {
        const FSkillData& SkillData = Pair.Value;
        // FText를 FString으로 변환하여 비교
        if (SkillData.SkillName.ToString().Equals(SkillName, ESearchCase::IgnoreCase))
        {
            return &SkillData;
        }
    }
    
    return nullptr;
}

bool UDiaSkillManager::CopySkillData(FSkillData& OutSkillData, int32 SkillID) const
{
    const FSkillData* skillData = SkillDataMap.Find(SkillID);
    if (skillData)
    {
        OutSkillData = *skillData;
        return true;
    }
    return false;
}

bool UDiaSkillManager::RegisterSkillData(int32 SkillID, const FSkillData& SkillData)
{
    if (SkillDataMap.Contains(SkillID))
    {
        return false;
    }

    SkillDataMap.Add(SkillID, SkillData);
    return true;
}

void UDiaSkillManager::UnregisterSkillData(int32 SkillID)
{
    SkillDataMap.Remove(SkillID);
}

void UDiaSkillManager::LoadDefaultSkillData()
{
    // 데이터 테이블 로드
    SkillDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *SkillDataTablePath));

    if (IsValid(SkillDataTable) == true)
    {
        TArray<FName> RowNames = SkillDataTable->GetRowNames();

        for (const FName& RowName : RowNames)
        {
            FDiaSkillTableRow* RowData = SkillDataTable->FindRow<FDiaSkillTableRow>(RowName, TEXT(""));
            if (RowData)
            {
                // FSkillData로 변환
                FSkillData SkillData;
                SkillData.SkillName = RowData->SkillName;
                SkillData.SkillDescription = RowData->SkillDescription;
                SkillData.SkillIcon = RowData->SkillIcon;
                SkillData.Cooldown = RowData->Cooldown;
                SkillData.ManaCost = RowData->ManaCost;
                SkillData.Damage = RowData->Damage;
                SkillData.SkillClass = RowData->SkillClass;

                // 스킬 ID는 RowName을 정수로 변환
                int32 SkillID = FCString::Atoi(*RowName.ToString());
                RegisterSkillData(SkillID, SkillData);
            }
        }
    }
}






