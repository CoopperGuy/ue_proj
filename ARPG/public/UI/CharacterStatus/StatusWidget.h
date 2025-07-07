// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatusWidget.generated.h"

class UTextBlock;
class UDiaStatComponent;

/**
 * 캐릭터 상태 UI 위젯
 * DiaStatComponent의 델리게이트를 바인딩하여 자동으로 스탯을 업데이트합니다.
 */
UCLASS()
class ARPG_API UStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯 초기화
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	// 스탯 컴포넌트와 바인딩 설정
	UFUNCTION(BlueprintCallable, Category = "Status")
	void BindToStatComponent(UDiaStatComponent* StatComponent);
	
	// 바인딩 해제
	UFUNCTION(BlueprintCallable, Category = "Status")
	void UnbindFromStatComponent();
	
	// Tick 기반 업데이트 활성화/비활성화
	UFUNCTION(BlueprintCallable, Category = "Status")
	void SetTickBasedUpdate(bool bEnabled, float UpdateInterval = 0.1f);
	
	// 상태 업데이트 함수 - 전체 스탯 업데이트
	UFUNCTION(BlueprintCallable, Category = "Status")
	void UpdateStatus(const FString& UserName, float CurrentHP, float MaxHP, float CurrentMP, float MaxMP, 
		float Strength, float Dexterity, float Intelligence, float AttackPower, float Defense, 
		int32 CurrentExp, int32 MaxExp);

	// 간단한 업데이트 함수들 (개별 스탯 업데이트용)
	UFUNCTION(BlueprintCallable, Category = "Status")
	void UpdateHealthMana(float CurrentHP, float MaxHP, float CurrentMP, float MaxMP);
	
	UFUNCTION(BlueprintCallable, Category = "Status")
	void UpdateExperience(int32 CurrentExp, int32 MaxExp);

	// 델리게이트 콜백 함수들
	UFUNCTION()
	void OnHealthChanged(float NewHealth, float MaxHealth);
	
	UFUNCTION()
	void OnManaChanged(float NewMana, float MaxMana);
	
	UFUNCTION()
	void OnExpChanged(float NewExp, float MaxExp);
	
	UFUNCTION()
	void OnLevelUp(int32 NewLevel);

protected:
	// Tick 기반 업데이트
	void TickBasedUpdate(float DeltaTime);
	
	// 현재 바인딩된 스탯 컴포넌트
	UPROPERTY()
	TWeakObjectPtr<UDiaStatComponent> BoundStatComponent;
	
	// Tick 기반 업데이트 설정
	UPROPERTY(EditAnywhere, Category = "Status|TickUpdate")
	bool bUseTickBasedUpdate = false;
	
	UPROPERTY(EditAnywhere, Category = "Status|TickUpdate")
	float TickUpdateInterval = 0.1f;
	
	UPROPERTY()
	float TickUpdateTimer = 0.0f;

public:
	// UI 요소들에 대한 참조 변수들
	UPROPERTY(meta = (BindWidget))
	UTextBlock* UserNameText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HPText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MPText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* StrText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DexText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* IntText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AtkText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ExpText;	
};
