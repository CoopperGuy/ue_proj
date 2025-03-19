// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Skill/DiaSkillBase.h"
#include "DiaProjectileSkill.generated.h"

class ADiaProjectile;

/**
 * 발사체 기반 스킬 클래스
 */
UCLASS()
class ARPG_API ADiaProjectileSkill : public ADiaSkillBase
{
	GENERATED_BODY()
	
public:
	ADiaProjectileSkill();

	// 스킬 실행
	virtual void ExecuteSkill() override;
	virtual void EndSkill() override;

	// 스킬 초기화
	virtual void InitializeSkill(AActor* InOwner) override;

protected:
	virtual void OnSkillStart() override;
	virtual void OnSkillEnd() override;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<ADiaProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	FVector ProjectileOffset;

	// 소유자 방향으로 발사할지 여부 (false면 마우스 커서 방향)
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	bool bUseOwnerRotation = false;

	// 다중 발사체 설정
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Multi")
	bool bFireMultipleProjectiles = false;

	// 발사체 개수
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Multi", meta = (EditCondition = "bFireMultipleProjectiles"))
	int32 ProjectileCount = 3;

	// 발사체 간 각도 (도 단위)
	UPROPERTY(EditDefaultsOnly, Category = "Projectile|Multi", meta = (EditCondition = "bFireMultipleProjectiles"))
	float SpreadAngle = 15.0f;

	// 발사체 생성 함수
	UFUNCTION()
	void SpawnProjectile();

};
