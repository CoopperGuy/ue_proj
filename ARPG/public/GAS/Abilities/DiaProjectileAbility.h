#pragma once

#include "CoreMinimal.h"
#include "GAS/DiaGameplayAbility.h"
#include "DiaProjectileAbility.generated.h"

class ADiaProjectile;

/**
 * 발사체 기반 GameplayAbility 클래스
 */
UCLASS()
class ARPG_API UDiaProjectileAbility : public UDiaGameplayAbility
{
	GENERATED_BODY()

public:
	UDiaProjectileAbility();

	virtual void InitializeWithSkillData(const FGASSkillData& InSkillData);

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnSpawned(AActor* SpawnedProjectile);
	UFUNCTION()
	void OnDidNotSpawn(AActor* SpawnedProjectile);
protected:
	// 발사체 생성 함수
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile();

	// 발사체 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	TSubclassOf<ADiaSkillActor> ProjectileClass;

	// 발사체 오프셋
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	FVector ProjectileOffset = FVector(50.0f, 0.0f, 0.0f);

	// 너무 가까운 지점을 클릭했을 때 보정에 사용할 최소 사거리
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float MinimumRange = 100.0f;

	// 소유자 방향으로 발사할지 여부 (false면 마우스 커서 방향)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	bool bUseOwnerRotation = false;

	// 다중 발사체 설정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Multi")
	bool bFireMultipleProjectiles = false;

	// 발사체 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Multi", meta = (EditCondition = "bFireMultipleProjectiles"))
	int32 ProjectileCount = 3;

	// 발사체 간 각도 (도 단위)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile|Multi", meta = (EditCondition = "bFireMultipleProjectiles"))
	float SpreadAngle = 15.0f;

private:
	// 발사 방향 계산
	FVector CalculateLaunchDirection(ACharacter* Character) const;

	// 마우스 커서 월드 위치 계산
	//FVector GetMouseWorldLocation() const;
};