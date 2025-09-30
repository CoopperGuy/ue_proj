// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonGameViewportClient.h"
#include "DiaCustomGameViewPort.generated.h"

class UItemDragDropOperation;
class UItemWidget;
class UDiaPrimaryLayout;
class UDragDropOperation;
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

	void CreateCautionWidget(UItemDragDropOperation* DragOp);
private:
	// 현재 드래그 상태 추적
	bool bIsDraggingItem = false;
	UItemDragDropOperation* CurrentDragOperation = nullptr;
	
	// 허공 드롭 처리 함수
	void HandleDropInVoid(const FIntPoint& MousePosition);
	
	// CreateCautionWidget 리팩토링된 헬퍼 함수들
	bool IsCautionWidgetAlreadyActive() const;
	class UDiaCaution* CreateAndInitializeCautionWidget(UItemDragDropOperation* DragOp);
	class UDiaInventoryComponent* GetInventoryComponent() const;
	void BindCautionWidgetEvents(class UDiaCaution* CautionWidget, const struct FInventorySlot& ItemData, class UDiaInventoryComponent* InvenComp);
	void DisplayCautionWidget(class UDiaCaution* CautionWidget);
	
	UPROPERTY()
	UDiaPrimaryLayout* DiaPrimaryLayout;
public:
	// 드래그 시작/종료 감지 함수들
	void OnDragStarted(UItemDragDropOperation* DragOp);
	void OnDragEnded();

	//아이템의 id를 넘겨서 해당 아이템을 delegate형식으로 삭제하도록 해보자. 
	//delegate를 인벤토리 위젯에 바인딩해야 할듯.
	void OnDragEnd_CreateCautionWidget();
};
