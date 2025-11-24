// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Types/ItemBase.h"
#include "Interface/ItemContainer.h"
#include "ItemDragDropOperation.generated.h"

class UItemWidget;
class IItemContainer;

UENUM(BlueprintType)
enum class EItemDragDropType : uint8
{
	EIDT_None      UMETA(DisplayName = "None"),
	EIDT_Inventory UMETA(DisplayName = "Inventory"),
	EIDT_Equipment UMETA(DisplayName = "Equipment"),
};
/**
 * 아이템 드래그 드롭 오퍼레이션
 */
UCLASS()
class ARPG_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	// 드래그되는 아이템 정보
	UPROPERTY(BlueprintReadOnly)
	FInventorySlot ItemData;
	
	// 원본 위젯 참조
	UPROPERTY(BlueprintReadOnly)
	UItemWidget* SourceWidget;
	
	// 원본 슬롯 인덱스
	UPROPERTY(BlueprintReadOnly)
	int32 SourceSlotIndex = -1;
	
	// 아이템 크기
	UPROPERTY(BlueprintReadOnly)
	int32 ItemWidth = 1;
	
	UPROPERTY(BlueprintReadOnly)
	int32 ItemHeight = 1;
	
	// 드래그 오프셋 (마우스 위치와 위젯 중심간의 차이)
	UPROPERTY(BlueprintReadOnly)
	FVector2D DragOffset = FVector2D::ZeroVector;

	// 드래그 타입
	UPROPERTY(BlueprintReadOnly)
	EItemDragDropType DragType = EItemDragDropType::EIDT_None;

	UPROPERTY(BlueprintReadOnly)
	TScriptInterface<IItemContainer> SourceContainer = nullptr;
}; 