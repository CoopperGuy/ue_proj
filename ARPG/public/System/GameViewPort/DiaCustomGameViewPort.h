// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonGameViewportClient.h"
#include "DiaCustomGameViewPort.generated.h"

class UItemDragDropOperation;
class UItemWidget;
class UDiaPrimaryLayout;
class UDragDropOperation;
class UDiaInstance;
/**
 * 
 */
UCLASS()
class ARPG_API UDiaCustomGameViewPort : public UCommonGameViewportClient
{
	GENERATED_BODY()
	
public:
	virtual void Init(struct FWorldContext& WorldContext, UGameInstance* OwningGameInstance, bool bCreateNewAudioDevice = true) override;
	virtual void Draw(FViewport* InViewport, FCanvas* SceneCanvas) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void LostFocus(FViewport* InViewport) override;
	
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;

	void SetupGameInstance(UDiaInstance* DiaInstance);

	// TODO: Caution 관련 로직 DiaController로 이전 예정
	// void CreateCautionWidget(UItemDragDropOperation* DragOp);
	// void OnDragStarted(UItemDragDropOperation* DragOp);
	// void OnDragEnded();
	// void OnDragEnd_CreateCautionWidget();

private:
	// TODO: 드래그 상태 추적 - DiaController로 이전 예정
	// bool bIsDraggingItem = false;
	// UItemDragDropOperation* CurrentDragOperation = nullptr;
	// void HandleDropInVoid(const FIntPoint& MousePosition);
	// bool IsCautionWidgetAlreadyActive() const;
	// class UDiaCaution* CreateAndInitializeCautionWidget(UItemDragDropOperation* DragOp);
	// class UDiaInventoryComponent* GetInventoryComponent() const;
	// void BindCautionWidgetEvents(class UDiaCaution* CautionWidget, const struct FInventorySlot& ItemData, class UDiaInventoryComponent* InvenComp);
	// void DisplayCautionWidget(class UDiaCaution* CautionWidget);

	UPROPERTY()
	TObjectPtr<UDiaPrimaryLayout> DiaPrimaryLayout;
public:
	UDiaPrimaryLayout* GetDiaPrimaryLayout() const { return DiaPrimaryLayout; }
};
