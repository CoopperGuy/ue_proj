// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "StatusWidget.generated.h"

class UTextBlock;
/**
 * 캐릭터 상태 UI 위젯 - GAS AttributeSet 기반
 */
UCLASS()
class ARPG_API UStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯 초기화 및 AttributeSet 바인딩
	virtual void NativeConstruct() override;
				
	void OnUpdateStats(const FOnAttributeChangeData& Data);

protected:
	// ASC 캐싱
	UPROPERTY()
	UAbilitySystemComponent* CachedASC = nullptr;

public:
	// UI 요소들 - Blueprint 위젯 바인딩
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
