#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DiaSkillType.generated.h"  
class ADiaSkillBase;
USTRUCT(BlueprintType)
struct ARPG_API FSkillData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText SkillName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText SkillDescription;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    UTexture2D* SkillIcon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Cooldown;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ManaCost;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Damage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<ADiaSkillBase> SkillClass;
};

//FSkillData를 기반으로 이용해 실질적 스킬 테이블 구성
USTRUCT(BlueprintType)
struct ARPG_API FDiaSkillBaseInfo : public FSkillData
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 SkillID;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SkillLevel;

    FDiaSkillBaseInfo(FSkillData InSkillData = FSkillData(), int32 InSkillID = -1, int32 InSkillLevel = 1)
        : FSkillData(InSkillData), SkillID(InSkillID), SkillLevel(InSkillLevel)
    {
	}
};