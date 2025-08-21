// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DiaCaution.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Animation/WidgetAnimation.h"

void UDiaCaution::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 버튼 클릭 이벤트 바인딩
    if (ConfirmButton)
    {
        ConfirmButton->OnClicked.AddDynamic(this, &UDiaCaution::OnClickConfirmButton);
    }
    
    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UDiaCaution::OnClickCancelButton);
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UDiaCaution::SetCautionText(const FText& Text)
{
    if (CautionText)
    {
        CautionText->SetText(Text);
    }
}

void UDiaCaution::BindOnConfirmClicked(const FOnConfirmClicked& InOnConfirmClicked)
{
    OnConfirmClickedDelegate = InOnConfirmClicked;
}

void UDiaCaution::BindOnCancelClicked(const FOnCancelClicked& InOnCancelClicked)
{
    OnCancelClickedDelegate = InOnCancelClicked;
}

void UDiaCaution::OnClickConfirmButton()
{
    //델리게이트가 바인딩되어 있으면 실행
    if (OnConfirmClickedDelegate.IsBound())
    {
        OnConfirmClickedDelegate.Execute();
    }
    
    //위젯 닫기, 애니메이션 재생 등
    CloseWidget();
}

void UDiaCaution::OnClickCancelButton()
{
    //델리게이트가 바인딩되어 있으면 실행
    if (OnCancelClickedDelegate.IsBound())
    {
        OnCancelClickedDelegate.Execute();
    }
    
    //위젯 닫기, 애니메이션 재생 등
    CloseWidget();
}

void UDiaCaution::CloseWidget()
{
    // 위젯을 닫는 로직
    //if (Anim_FadeOut)
    //{
    //    PlayAnimation(Anim_FadeOut);
    //}
    
    // 위젯을 숨김 처리
	SetVisibility(ESlateVisibility::Hidden);
}




