// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Skill/Effect/DiaStatusEffect.h"
#include "DiaStatusEffectComponent.generated.h"

class UDiaStatusEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectAddedDelegate, UDiaStatusEffect*, StatusEffect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectRemovedDelegate, UDiaStatusEffect*, StatusEffect);

/**
 * 캐릭터의 상태 이상 효과를 관리하는 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARPG_API UDiaStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDiaStatusEffectComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 상태 효과 추가
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	UDiaStatusEffect* AddStatusEffect(TSubclassOf<UDiaStatusEffect> StatusEffectClass, float Duration = -1.0f, float Strength = -1.0f, AActor* Instigator = nullptr);

	// ID로 상태 효과 추가
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	UDiaStatusEffect* AddStatusEffectByID(const FString& StatusEffectID, float Duration = -1.0f, float Strength = -1.0f, AActor* Instigator = nullptr);

	// 상태 효과 제거
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	bool RemoveStatusEffect(UDiaStatusEffect* StatusEffect);

	// ID로 상태 효과 제거
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	bool RemoveStatusEffectByID(const FString& StatusEffectID);

	// 태그로 상태 효과 제거
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	int32 RemoveStatusEffectsByTag(const FName& Tag);

	// 모든 상태 효과 제거
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void RemoveAllStatusEffects();

	// 상태 효과 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	bool HasStatusEffect(TSubclassOf<UDiaStatusEffect> StatusEffectClass) const;

	// ID로 상태 효과 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	bool HasStatusEffectByID(const FString& StatusEffectID) const;

	// 태그로 상태 효과 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	bool HasStatusEffectWithTag(const FName& Tag) const;

	// ID로 상태 효과 가져오기
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	UDiaStatusEffect* GetStatusEffectByID(const FString& StatusEffectID) const;

	// 모든 상태 효과 가져오기
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	TArray<UDiaStatusEffect*> GetAllStatusEffects() const;

	// 태그로 상태 효과 가져오기
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	TArray<UDiaStatusEffect*> GetStatusEffectsByTag(const FName& Tag) const;

	// 이벤트 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "StatusEffects")
	FOnStatusEffectAddedDelegate OnStatusEffectAdded;

	UPROPERTY(BlueprintAssignable, Category = "StatusEffects")
	FOnStatusEffectRemovedDelegate OnStatusEffectRemoved;

protected:
	// 활성화된 상태 효과 목록
	UPROPERTY()
	TArray<UDiaStatusEffect*> ActiveStatusEffects;

	// 상태 효과 클래스 맵 (ID -> 클래스)
	UPROPERTY(EditDefaultsOnly, Category = "Status Effects")
	TMap<FString, TSubclassOf<UDiaStatusEffect>> StatusEffectClassMap;
};
