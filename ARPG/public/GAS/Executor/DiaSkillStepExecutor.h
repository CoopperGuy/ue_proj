// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/DiaGASSkillData.h"
#include "DiaSkillStepExecutor.generated.h"

class ADiaSkillActor;
USTRUCT(BlueprintType)
struct ARPG_API FDiaSkillExecutionContext
{
	GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> Instigator = nullptr;
    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> Target = nullptr;
    UPROPERTY(BlueprintReadOnly)
	TArray<TObjectPtr<ADiaSkillActor>> SpawnedActors;
	UPROPERTY(BlueprintReadOnly)
	FVector LastStepLocation = FVector::ZeroVector;
};

class UDiaGameplayAbility;
/**
 * 
 */
UCLASS(Abstract)
class ARPG_API UDiaSkillStepExecutor : public UObject
{
	GENERATED_BODY()
public:
    virtual bool CanExecute(const FInstancedStruct& StepData) const { return false; }

    virtual void Execute(
        const FInstancedStruct& StepData,
        UDiaGameplayAbility* Ability,
        FDiaSkillExecutionContext& Context,
        TFunction<void()> OnFinished) {}
};

UCLASS()
class ARPG_API UDiaChargeStepExecutor : public UDiaSkillStepExecutor
{
	GENERATED_BODY()
public:
	virtual bool CanExecute(const FInstancedStruct& StepData) const override;

    virtual void Execute(
        const FInstancedStruct& StepData,
        UDiaGameplayAbility* Ability,
        FDiaSkillExecutionContext& Context,
		TFunction<void()> OnFinished) override;

    FVector CalcSweepPosition(const FGameplayAbilityActorInfo& ActorInfo);

	UFUNCTION()
	void OnDashFinished();

private:
    UPROPERTY()
	FVector StartLoc = FVector::ZeroVector;
	UPROPERTY()
	FVector EndLoc = FVector::ZeroVector;

    UPROPERTY()
	UDiaGameplayAbility* CachedAbility = nullptr;

	TFunction<void()> CacheOnFinished;
};

UCLASS()
class ARPG_API UDiaGroundSpawnStepExecutor : public UDiaSkillStepExecutor
{
	GENERATED_BODY()
public:
	virtual bool CanExecute(const FInstancedStruct& StepData) const override;
    virtual void Execute(
        const FInstancedStruct& StepData,
        UDiaGameplayAbility* Ability,
        FDiaSkillExecutionContext& Context,
		TFunction<void()> OnFinished) override;
};

UCLASS()
class ARPG_API UDiaMeleeSpawnStepExecutor : public UDiaSkillStepExecutor
{
	GENERATED_BODY()
public:
	virtual bool CanExecute(const FInstancedStruct& StepData) const override;
    virtual void Execute(
        const FInstancedStruct& StepData,
        UDiaGameplayAbility* Ability,
        FDiaSkillExecutionContext& Context,
		TFunction<void()> OnFinished) override;
};

UCLASS()
class ARPG_API UDiaProjectileSpawnStepExecutor : public UDiaSkillStepExecutor
{
	GENERATED_BODY()
public:
	virtual bool CanExecute(const FInstancedStruct& StepData) const override;
    virtual void Execute(
        const FInstancedStruct& StepData,
        UDiaGameplayAbility* Ability,
        FDiaSkillExecutionContext& Context,
		TFunction<void()> OnFinished) override;

	FVector CalcSpawnLocation(const FGameplayAbilityActorInfo& ActorInfo, const FGASProjectileData* ProjectileData);
};
