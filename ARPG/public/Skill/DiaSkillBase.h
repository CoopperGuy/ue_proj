// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Skill/DiaSkillType.h"
#include "Skill/Effect/DiaStatusEffect.h"
#include "DiaSkillBase.generated.h"

class UNiagaraSystem;
class UAnimMontage;
class ADiaBaseCharacter;
            
UCLASS(Abstract, Blueprintable)
class ARPG_API ADiaSkillBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADiaSkillBase();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

	// 스킬 실행 및 종료
	UFUNCTION()
	virtual void ExecuteSkill();

	UFUNCTION()
	virtual void EndSkill();

	// 스킬 초기화
	UFUNCTION()
	virtual void InitializeSkill(AActor* InOwner);

	// 스킬 실행 가능 여부 체크 함수 추가
	UFUNCTION(BlueprintCallable, Category = "Skill")
	virtual bool CanExecuteSkill() const;

	// 스킬 데이터로 초기화
	virtual void InitializeWithData(const FSkillData* SkillData);
    
protected:
	// OnSkillStart/End 함수들을 가상 함수로 만들어 자식 클래스에서 오버라이드 할 수 있도록 함
	virtual void OnSkillStart();
	virtual void OnSkillEnd();

	// 타이머 핸들 추가
	FTimerHandle CooldownTimerHandle;

	// 스킬 실행 중인지 여부 추가
	UPROPERTY(BlueprintReadOnly, Category = "Skill State")
	bool bIsExecuting;

protected:
	// 스킬 소유자
	UPROPERTY()
	ADiaBaseCharacter* SkillOwner;

	// 스킬 데이터
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill Data")
	FSkillData SkillData;

	// 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	UNiagaraSystem* SkillEffect;

	// 애니메이션
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* SkillMontage;

	// 스킬 상태
	UPROPERTY(BlueprintReadOnly, Category = "Skill State")
	bool bIsOnCooldown;

	// 스킬 사용 시 적용할 상태 이상 효과
	UPROPERTY(EditDefaultsOnly, Category = "Status Effects")
	TArray<TSubclassOf<UDiaStatusEffect>> StatusEffects;
	
	// 상태 이상 효과 적용 확률 (0.0-1.0)
	UPROPERTY(EditDefaultsOnly, Category = "Status Effects")
	float StatusEffectChance = 1.0f;

public:
	FORCEINLINE float GetDamage() const { return SkillData.Damage; }
	FORCEINLINE float GetCooldown() const { return SkillData.Cooldown; }
	FORCEINLINE float GetManaCost() const { return SkillData.ManaCost; }
	FORCEINLINE FText GetSkillName() const { return SkillData.SkillName; }
	FORCEINLINE FText GetSkillDescription() const { return SkillData.SkillDescription; }
	FORCEINLINE UTexture2D* GetSkillIcon() const { return SkillData.SkillIcon; }
	const FSkillData& GetSkillData() const { return SkillData; }

	// 대상에게 상태 이상 효과 적용
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void ApplyStatusEffectsToTarget(AActor* Target);
};
