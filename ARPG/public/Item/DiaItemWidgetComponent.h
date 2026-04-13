// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DiaItemWidgetComponent.generated.h"

/**
 * UWidgetComponent::bReceiveHardwareInput 가 protected 인 엔진에서,
 * 서브클래스 생성자/InitWidget 안에서만 true 로 켤 수 있음.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ARPG_API UDiaItemWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UDiaItemWidgetComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** InitWidget 등 이후에도 엔진이 false 로 되돌릴 수 있어 필요 시 명시 호출 */
	void EnsureReceiveHardwareInput();

	/** 외부(ADiaItem 등)에서 protected 멤버 대신 계측용으로 사용 */
	UFUNCTION(BlueprintPure, Category = "Widget")
	bool IsReceiveHardwareInputEnabled() const { return bReceiveHardwareInput; }
};
