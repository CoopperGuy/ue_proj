// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "DiaCustomGameViewPort.generated.h"

class UItemDragDropOperation;
class UItemWidget;
class UDiaPrimaryLayout;

/**
 * 
 */
UCLASS()
class ARPG_API UDiaCustomGameViewPort : public UGameViewportClient
{
	GENERATED_BODY()
	
public:
	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
	virtual void Draw(FViewport* InViewport, FCanvas* SceneCanvas) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void LostFocus(FViewport* InViewport) override;
	
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;

	void CreateCautionWidget();
private:
	// 현재 드래그 상태 추적
	bool bIsDraggingItem = false;
	UItemDragDropOperation* CurrentDragOperation = nullptr;
	
	// 허공 드롭 처리 함수
	void HandleDropInVoid(const FIntPoint& MousePosition);
	
	UPROPERTY()
	UDiaPrimaryLayout* DiaPrimaryLayout;
public:
	// 드래그 시작/종료 감지 함수들
	void OnDragStarted(UItemDragDropOperation* DragOp);
	void OnDragEnded();
};
