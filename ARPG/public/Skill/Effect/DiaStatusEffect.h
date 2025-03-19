// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DiaStatusEffect.generated.h"

class ADiaBaseCharacter;

UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
	Buff,           // 긍정적 효과
	Debuff,         // 부정적 효과
	ControlImpair,  // 제어 효과 (스턴, 슬로우 등)
	DamageOverTime, // 지속 데미지
	Heal,           // 지속 회복
	Other           // 기타
};

/**
 * 캐릭터에 적용되는 상태 이상/버프 효과의 기본 클래스
 */
UCLASS(Abstract, Blueprintable)
class ARPG_API UDiaStatusEffect : public UObject
{
	GENERATED_BODY()
	
public:
	UDiaStatusEffect();
	
	// 효과 초기화
	virtual void Initialize(ADiaBaseCharacter* InOwner, float InDuration, float InStrength);
	
	// 효과 적용/해제
	virtual void Apply();
	virtual void Remove();
	
	// 효과 틱 처리
	virtual void Tick(float DeltaTime);
	
	// 상태 확인
	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	bool IsActive() const { return bIsActive; }
	
	// 식별 정보
	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	FString GetStatusEffectID() const { return StatusEffectID; }
	
	// 남은 시간 조회
	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	float GetRemainingTime() const { return RemainingTime; }
	
	// 태그 확인
	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	bool HasTag(const FName& Tag) const;
	
	// 효과 태그 (효과 종류 상세 분류)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	TArray<FName> Tags;

	// Getters
	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	FText GetDescription() const { return Description; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	UTexture2D* GetIcon() const { return Icon; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	EStatusEffectType GetEffectType() const { return EffectType; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	float GetDuration() const { return Duration; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	float GetStrength() const { return Strength; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	bool CanStack() const { return bCanStack; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	int32 GetMaxStacks() const { return MaxStacks; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	int32 GetCurrentStacks() const { return CurrentStacks; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	ADiaBaseCharacter* GetOwnerCharacter() const { return OwnerCharacter; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	AActor* GetInstigator() const { return Instigator; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	float GetTickInterval() const { return TickInterval; }

	// Setters
	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetDisplayName(const FText& NewDisplayName) { DisplayName = NewDisplayName; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetDescription(const FText& NewDescription) { Description = NewDescription; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetIcon(UTexture2D* NewIcon) { Icon = NewIcon; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetEffectType(EStatusEffectType NewEffectType) { EffectType = NewEffectType; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetDuration(float NewDuration) { Duration = NewDuration; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetStrength(float NewStrength) { Strength = NewStrength; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetCanStack(bool bNewCanStack) { bCanStack = bNewCanStack; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetMaxStacks(int32 NewMaxStacks) { MaxStacks = NewMaxStacks; }

	// 스택 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	bool AddStack();

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	bool RemoveStack();

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetCurrentStacks(int32 NewStacks);

	// 효과 소유자 및 적용자 설정
	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetOwnerCharacter(ADiaBaseCharacter* NewOwner) { OwnerCharacter = NewOwner; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetInstigator(AActor* NewInstigator) { Instigator = NewInstigator; }

	UFUNCTION(BlueprintCallable, Category = "Status Effect")
	void SetTickInterval(float NewTickInterval) { TickInterval = NewTickInterval; }

protected:
	// 효과 ID (고유 식별자)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	FString StatusEffectID;
	
	// 효과 이름 (표시용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	FText DisplayName;
	
	// 효과 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	FText Description;
	
	// 효과 아이콘
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	UTexture2D* Icon;
	
	// 효과 종류
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	EStatusEffectType EffectType;	
	// 지속 시간 (-1은 무제한)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	float Duration;
	
	// 효과 강도
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	float Strength;
	
	// 중첩 여부
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	bool bCanStack;
	
	// 중첩 최대 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect", meta = (EditCondition = "bCanStack"))
	int32 MaxStacks;
	
	// 현재 중첩 수
	UPROPERTY(BlueprintReadOnly, Category = "Status Effect")
	int32 CurrentStacks;
	
	// 효과 소유자
	UPROPERTY()
	ADiaBaseCharacter* OwnerCharacter;
	
	// 효과 적용자
	UPROPERTY()
	AActor* Instigator;
	
	// 현재 활성화 여부
	UPROPERTY(BlueprintReadOnly, Category = "Status Effect")
	bool bIsActive;
	
	// 남은 시간
	UPROPERTY(BlueprintReadOnly, Category = "Status Effect")
	float RemainingTime;
	
	// 주기적 효과 틱 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Status Effect")
	float TickInterval;
	
	// 마지막 틱 이후 경과 시간
	float TimeSinceLastTick;
	
	// 효과 틱 발동
	virtual void OnEffectTick();
};
