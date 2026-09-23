#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "DiaGASSkillExtraData.generated.h"

class ADiaSkillActor;
class UCurveVector;

/**
 * FGASSkillData::ExtraData 의 폴리모픽 베이스.
 * 최상위 ExtraData 필드는 레거시 호환용이다. 신규 스킬은 FGASSkillData::Steps를 사용한다.
 * 이 파일의 Projectile/Ground/Charge 등의 설정 구조체는 StepData 내부에서도 재사용한다.
 * 에디터에서 FInstancedStruct 의 BaseStruct 메타로 이 타입을 지정하면
 * DataTable 행에서 파생 구조체를 콤보로 선택할 수 있다.
 */
USTRUCT(BlueprintType)
struct ARPG_API FGASSkillExtraDataBase
{
    GENERATED_BODY()
};

/**
 * Projectile/Ground 처럼 월드에 ADiaSkillActor 를 스폰하는 스킬 공통 설정.
 * Step 내부 스폰 설정에도 재사용하며, 레거시 ExtraData 호환에서도 이 타입의 LifeSpan을 읽는다.
 */
USTRUCT(BlueprintType)
struct ARPG_API FGASSkillActorSpawnData : public FGASSkillExtraDataBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<ADiaSkillActor> SkillActorClass;

    /** 스킬 오브젝트가 월드에 남는 시간(초). 0 이하면 엔진 LifeSpan 기본 동작을 따름. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0"))
    float LifeSpan = 0.0f;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASProjectileData : public FGASSkillActorSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    FVector ProjectileOffset = FVector(50.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    float MinimumRange = 100.0f;

    /** false 이면 마우스 커서 방향, true 이면 소유자(캐릭터) 정면으로 발사 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
    bool bUseOwnerRotation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Multi")
    bool bFireMultipleProjectiles = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Multi", meta = (EditCondition = "bFireMultipleProjectiles", ClampMin = "1"))
    int32 ProjectileCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Multi", meta = (EditCondition = "bFireMultipleProjectiles", ClampMin = "0.0"))
    float SpreadAngle = 15.0f;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASGroundData : public FGASSkillActorSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground")
    bool bUseOwnerRotation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground", meta = (ClampMin = "0.0"))
    float Radius = 0.0f;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASMeleeData : public FGASSkillActorSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee", meta = (ClampMin = "0.0", ClampMax = "360.0"))
    float AttackAngle = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
    FVector AttackOffset = FVector(100.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee|Debug")
    bool bShowDebugShape = true;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASUtilitySpawnData : public FGASSkillActorSpawnData
{
    GENERATED_BODY()

    /** Optional second cosmetic actor. Used by skills such as teleport arrival. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
    TSubclassOf<ADiaSkillActor> SecondarySkillActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
    FVector SpawnOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
    bool bUseOwnerRotation = true;

    /** Attach cosmetic actors to the avatar for auras/trails. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility")
    bool bAttachToOwner = false;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASChargeData : public FGASSkillExtraDataBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (ClampMin = "0.0"))
    float ChargeDelay = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (ClampMin = "0.0"))
    float ChargeDuration = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (ClampMin = "0.0"))
    float UpdateTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge", meta = (ClampMin = "0.0"))
    float ChargeDist = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Charge")
    TObjectPtr<UCurveVector> PathOffsetCurve = nullptr;
};

USTRUCT(BlueprintType)
struct ARPG_API FGASDodgeData : public FGASSkillExtraDataBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (ClampMin = "0.0"))
    float DodgeStrength = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge", meta = (ClampMin = "0.0"))
    float DodgeDuration = 0.5f;
};

