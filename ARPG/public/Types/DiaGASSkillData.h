#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilities/Public/GameplayAbilitySpec.h"
#include "DiaGASSkillData.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class ADiaProjectile;

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
    TSubclassOf<UGameplayAbility> AbilityClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<ADiaProjectile> ProjectileClass;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Range = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 0.0f;

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


    FGASSkillData()
    {
        SkillID = 0;
        SkillName = FText::FromString(TEXT("Unknown Skill"));
        Description = FText::FromString(TEXT("No Description"));
        BaseDamage = 0.0f;
        ManaCost = 0.0f;
        CooldownDuration = 0.0f;
        CastTime = 0.0f;
        Range = 0.0f;
        Radius = 0.0f;
        MaxLevel = 1;
        RequiredCharacterLevel = 1;
    }
};