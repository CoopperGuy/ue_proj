#pragma once

#include "CoreMinimal.h"
#include "Types/DiaGASSkillExtraData.h"
#include "DiaGASSkillStepData.generated.h"

/**
 * FGASSkillData::Steps 의 폴리모픽 베이스.
 * 신규 스킬은 단일 동작도 StepData로 작성하며, 복합 스킬은 여러 StepData를 순서대로 실행한다.
 * 각 단계의 실행은 UDiaSkillStepExecutor가 담당한다. 기존 전용 Ability에 신규 동작을 추가하는 방식은 지양한다.
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

	/** Active hit time. One target can be damaged only once per step. Zero is an instant hit. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float HitWindowDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
	bool bUseLastStepEndLocation = true;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASUtilityStepData : public FGASSkillStepDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
	FGASUtilitySpawnData UtilityData;
};
