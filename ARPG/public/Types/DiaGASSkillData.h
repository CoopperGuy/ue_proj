#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilities/Public/GameplayAbilitySpec.h"
#include "StructUtils/InstancedStruct.h"
#include "Types/DiaGASSkillExtraData.h"
#include "Types/DiaGASSkillStepData.h"
#include "DiaGASSkillData.generated.h"

class UGameplayAbility;
class UDiaGameplayAbility;
class UGameplayEffect;
class ADiaProjectile;
class ADiaSkillActor;

// GAS 스킬 타입
UENUM(BlueprintType)
enum class EGASSkillType : uint8
{
    None = 0,
    MeleeAttack,        // 근접 공격
    RangedAttack,       // 원거리 공격
    Magic,              // 마법
    Buff,               // 버프
    Debuff,             // 디버프
    Heal,               // 치유
    Movement,           // 이동 관련
    Dodge,              // 회피
    Passive             // 패시브
};

// GAS 스킬 대상 타입
UENUM(BlueprintType)
enum class EGASTargetType : uint8
{
    None = 0,
    Self,               // 자신
    Enemy,              // 적
    Ally,               // 아군
    Ground,             // 지면
    Direction           // 방향
};

// GAS 스킬 데이터
USTRUCT(BlueprintType)
struct ARPG_API FGASSkillData : public FTableRowBase
{
    GENERATED_BODY()

    // 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> Icon;

    // GAS 관련 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UDiaGameplayAbility> AbilityClass;

    /**
     * 기존 단일 실행 스킬용 추가 데이터(Projectile/Melee/Ground/Charge/Dodge 등).
     * 신규 복합 스킬은 Steps 사용을 우선한다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extra", meta = (BaseStruct = "/Script/ARPG.GASSkillExtraDataBase"))
    FInstancedStruct ExtraData;

    /**
     * 복합 스킬 실행 단계.
     * 예: ChargeStep -> GroundSpawnStep 처럼 여러 실행부를 데이터로 조합한다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steps", meta = (BaseStruct = "/Script/ARPG.GASSkillStepDataBase"))
    TArray<FInstancedStruct> Steps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGASSkillType SkillType = EGASSkillType::MeleeAttack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGASTargetType TargetType = EGASTargetType::Enemy;

    // 스킬 태그
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer AbilityTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer CancelAbilitiesWithTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer BlockAbilitiesWithTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag ManaCostTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag CoolDownTags;

    // 스킬 수치
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ManaCost = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CastTime = 0.0f;

    // 다단 히트 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi Hit")
    int32 HitCount = 1; // 타격 횟수 (1 = 단타, 2+ = 다단히트)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi Hit")
    float HitInterval = 0.0f; // 타격 간격 (초 단위, 0 = 즉시)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi Hit")
    float DamageMultiplierPerHit = 1.0f; // 각 히트별 데미지 배율 (예: 0.5 = 50%)

    // 애니메이션 및 이펙트
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimMontage> CastAnimation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<class UNiagaraSystem> CastEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<class UParticleSystem> LegacyCastEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundBase> CastSound;

    // 레벨 관련
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredCharacterLevel = 1;

    // 전제 조건 스킬
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> PrerequisiteSkills;

    // GameplayEffect 클래스들
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<UGameplayEffect>> EffectsToApplyOnHit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSubclassOf<UGameplayEffect>> EffectsToApplyOnSelf;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> VariantIDs;

    FGASSkillData()
    {
        SkillID = 0;
        SkillName = FText::FromString(TEXT("Unknown Skill"));
        Description = FText::FromString(TEXT("No Description"));
        BaseDamage = 0.0f;
        ManaCost = 0.0f;
        CooldownDuration = 0.0f;
        CastTime = 0.0f;
        MaxLevel = 1;
        RequiredCharacterLevel = 1;
        HitCount = 1;
        HitInterval = 0.0f;
        DamageMultiplierPerHit = 1.0f;

        VariantIDs.Reserve(3);
        EffectsToApplyOnHit.Reserve(6);
        EffectsToApplyOnSelf.Reserve(6);
        PrerequisiteSkills.Reserve(3);
    }

    /**
     * ExtraData 에서 특정 파생 타입을 안전하게 읽는 헬퍼.
     * 타입이 일치하거나 파생 관계이면 포인터, 아니면 nullptr.
     */
    template<typename T>
    const T* GetExtraPtr() const
    {
        static_assert(TIsDerivedFrom<T, FGASSkillExtraDataBase>::IsDerived,
            "T must derive from FGASSkillExtraDataBase");
        return ExtraData.GetPtr<T>();
    }

    template<typename T>
    const T* GetStepPtr(int32 StepIndex) const
    {
        static_assert(TIsDerivedFrom<T, FGASSkillStepDataBase>::IsDerived,
            "T must derive from FGASSkillStepDataBase");
        return Steps.IsValidIndex(StepIndex) ? Steps[StepIndex].GetPtr<T>() : nullptr;
    }
};

UENUM(BlueprintType)
enum class EJobType : uint8
{
    Warrior,
    JobType_MAX
};


USTRUCT(BlueprintType)
struct ARPG_API FJobSkillSet : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EJobType JobType = EJobType::Warrior;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> SkillIDs;
};

USTRUCT(BlueprintType)
struct ARPG_API FSkillVariantData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VariantID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VariantName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ModifierValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag VariantTag;

    FSkillVariantData()
    {
        VariantID = 0;
        VariantName = FText::FromString(TEXT("Unknown Variant"));
        Description = FText::FromString(TEXT("No Description"));
        ModifierValue = 1.0f;
        VariantTag = FGameplayTag();
	}
};
