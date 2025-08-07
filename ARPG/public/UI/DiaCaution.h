// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiaCaution.generated.h"

class UTextBlock;
class UButton;
class UWidgetAnimation;
class UWidgetTree;
class UButton;

// 델리게이트 선언 - 버튼 클릭 시 실행할 함수들을 바인딩하기 위함
DECLARE_DELEGATE(FOnConfirmClicked);
DECLARE_DELEGATE(FOnCancelClicked);

/**
 * 
 */
UCLASS()
class ARPG_API UDiaCaution : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;
	void SetCautionText(const FText& Text);

	// 델리게이트 바인딩 함수들 - 외부에서 실행할 함수를 등록
	void BindOnConfirmClicked(const FOnConfirmClicked& InOnConfirmClicked);
	void BindOnCancelClicked(const FOnCancelClicked& InOnCancelClicked);

	UFUNCTION()
	void OnClickConfirmButton();
	UFUNCTION()
	void OnClickCancelButton();
private:
	void CloseWidget();
public:
	// 델리게이트 인스턴스들
	FOnConfirmClicked OnConfirmClickedDelegate;
	FOnCancelClicked OnCancelClickedDelegate;

protected:
	UPROPERTY(meta = (BindWidget))
	UWidgetAnimation* Anim_FadeOut;

	UPROPERTY(meta = (BindWidget))
	UWidgetAnimation* Anim_FadeIn;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CautionText;

	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CancelButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ConfirmButtonText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CancelButtonText;
};
