// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/DiaSkillActor.h"
#include "DiaGroundObj.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API ADiaGroundObj : public ADiaSkillActor
{
	GENERATED_BODY()
	
public:
	ADiaGroundObj();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Initialize(const FGASSkillData& SkillData, AActor* InOwner, UAbilitySystemComponent* InSourceASC = nullptr, TSubclassOf<UGameplayEffect> InDamageEffect = nullptr) override;
	virtual void ApplySpawnRuntimeParams(const FDiaSkillRuntimeParams& InParams);
	virtual void ConfigureSkillActor(const FDiaSkillActorConfigureData& Config) override;

protected:
	void OnHitDetect();
	void StartHitDetectTimer();
	void StopHitDetectTimer();
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skill|Ground")
	float Radius = 200.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skill|Ground")
	float Radius_BM = 0.f;

	UPROPERTY()
	FTimerHandle HitDetectTimerHandle;

public:
	void SetRadius(float InRadius) { Radius = InRadius; }
	float GetRadius() const { return Radius; }
	void SetRadius_BM(float InRadius_BM) { Radius_BM = InRadius_BM; }
	float GetRadius_BM() const { return Radius_BM; }
	float GetTotalRadius() const { return Radius + Radius_BM; }
};
