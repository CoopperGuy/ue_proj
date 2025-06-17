// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "ItemWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class ARPG_API UItemWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
	// 드래그 앤 드롭 지원
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	void SetItemInfo(const FInventoryItem& ItemData);
    void SetWidgetGridPos(int32 PositionX, int32 PositionY);
	
	// Getter 함수들
	FORCEINLINE const FInventoryItem& GetItemInfo() const { return ItemInfo; }	
protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(BlueprintReadOnly)
	FInventoryItem ItemInfo;
};
