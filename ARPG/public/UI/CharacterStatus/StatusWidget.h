// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StatusWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class ARPG_API UStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 위젯 초기화
	virtual void NativeConstruct() override;
	// 상태 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "Status")
	void UpdateStatus(float CurrentHP, float MaxHP, float CurrentMP, float MaxMP);
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
