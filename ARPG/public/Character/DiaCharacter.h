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
class ARPG_API ADiaCharacter : public ADiaBaseCharacter, public ICombatable
{
	GENERATED_BODY()

public:
	ADiaCharacter();

    bool GetMouseWorldLocation(FVector& OutLocation) const;
    //Method
protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Input Action Functions
    void Move(const FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    // 공격 입력 처리
    void ExecuteSkillByIndex(int32 ActionIndex);
    // 콤보 처리
    void UpdateCharacterRotation();

    // 어택 가능 여부
    virtual bool CanAttack() const;

    // 공격 여부
    virtual bool IsInCombat() const;

    // 초기 스킬 설정
    virtual void SetupInitialSkills() override;
public:	
	virtual void Tick(float DeltaTime) override;

    // 공격 처리 함수
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
protected:
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

    // 카메라 암
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    USpringArmComponent* CameraBoom;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
    UCameraComponent* FollowCamera;
    
    UPROPERTY(EditAnywhere, Category = "Movement")
    float RotationInterpSpeed = 15.0f;
    // 스킬 ID 매핑 (키 인덱스 -> 스킬 ID)
    UPROPERTY(EditDefaultsOnly, Category = "Skills")
    TArray<int32> SkillIDMapping;

    const int32 MaxSkillMapping = 8;
};
