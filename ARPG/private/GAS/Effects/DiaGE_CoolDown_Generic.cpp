// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Effects/DiaGE_CoolDown_Generic.h"
#include "GAS/DiaAttributeSet.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UDiaGE_CoolDown_Generic::UDiaGE_CoolDown_Generic()
{
	// - 지속형(HasDuration): 실제 지속시간은 적용 시점에 Spec에서 주입합니다.
	// - 모디파이어/주기 없음: 쿨다운은 태그만 부여하고 수치를 변경하지 않습니다.
	// - 스택 정책: 중복 불가, 재적용 시 남은 시간 갱신

    DurationPolicy = EGameplayEffectDurationType::HasDuration;

    // SetByCaller 방식으로 Duration 설정 - GASData.CoolDown 태그로 값 주입
    FSetByCallerFloat SetByCallerDuration;
    SetByCallerDuration.DataTag = FGameplayTag::RequestGameplayTag(FName("GASData.CoolDown"));
    DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCallerDuration);

    Period = FScalableFloat(0.0f);

    Modifiers.Reset();

    StackLimitCount = 1;
    bDenyOverflowApplication = true;
    StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;
    StackPeriodResetPolicy = EGameplayEffectStackingPeriodPolicy::NeverReset;
}
