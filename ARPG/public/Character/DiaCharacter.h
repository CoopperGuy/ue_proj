// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Interface/Combatable.h"
#include "DiaBaseCharacter.h"
#include "DiaCharacter.generated.h"

class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class UDiaCombatComponent;
class ADiaSkillBase;
UCLASS()
class ARPG_API ADiaCharacter : public ADiaBaseCharacter
{
	GENERATED_BODY()

public:
	ADiaCharacter();


    /// <summary>
    /// 애니메이션 관련 처리
    /// </summary>
    virtual void PlayDieAnimation();
    virtual void Die();

    /// <summary>
    /// UI관련 함수
    /// </summary>
    virtual void UpdateHPGauge(float CurHealth, float MaxHelath);

    // 움직임 관련 함수 
    bool GetMouseWorldLocation(FVector& OutLocation) const;
protected:
	/// <summary>
	/// 엔진 기본 함수
	/// </summary>
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Tick(float DeltaTime) override;

	/// <summary>
	/// 움직임 관련 함수
    /// </summary>
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void ExecuteSkillByIndex(int32 ActionIndex);

    // 공격 입력 처리
    void UpdateCharacterRotation();


    /// <summary>
    /// 전투 관련 함수
    /// </summary>
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    
    // 초기 스킬 설정
    virtual void SetupInitialSkills() override;

protected:
    /// 입력 관련 변수
    // Enhanced Input Actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* MovementAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* LookAction;
    
    // Input Actions for Skills
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Skills")
    TArray<UInputAction*> SkillActions;

    // Input Mapping Context
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputMappingContext* DefaultMappingContext;

    ///화면 관련 변수
    // 카메라 암
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    USpringArmComponent* CameraBoom;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    UCameraComponent* FollowCamera;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float RotationInterpSpeed = 15.0f;

    //스킬 관련 변수
    // 스킬 ID 매핑 (키 인덱스 -> 스킬 ID)
    UPROPERTY(EditDefaultsOnly, Category = "Skills")
    TArray<int32> SkillIDMapping;

    const int32 MaxSkillMapping = 8;
};
