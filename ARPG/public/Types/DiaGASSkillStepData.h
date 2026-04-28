#pragma once

#include "CoreMinimal.h"
#include "Types/DiaGASSkillExtraData.h"
#include "DiaGASSkillStepData.generated.h"

/**
 * FGASSkillData::Steps 의 폴리모픽 베이스.
 * 복합 스킬은 여러 StepData를 순서대로 실행한다.
 */
USTRUCT(BlueprintType)
struct ARPG_API FGASSkillStepDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step", meta = (ClampMin = "0.0"))
	float StartDelay = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Step")
	bool bWaitUntilFinished = true;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASChargeStepData : public FGASSkillStepDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge")
	FGASChargeData ChargeData;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASGroundSpawnStepData : public FGASSkillStepDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	FGASGroundData GroundData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	bool bUseLastStepEndLocation = true;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASProjectileSpawnStepData : public FGASSkillStepDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	FGASProjectileData ProjectileData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	bool bUseLastStepEndLocation = true;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASMeleeSpawnStepData : public FGASSkillStepDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	FGASMeleeData MeleeData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	bool bUseLastStepEndLocation = true;
};
