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