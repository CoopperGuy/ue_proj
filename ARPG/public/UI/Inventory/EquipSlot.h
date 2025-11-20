// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "EquipSlot.generated.h"

class UImage;
class UItemWidget;
class USizeBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquipped, const FEquippedItem&, Item, EEquipmentSlot, SlotType);

/**
 * 
 */
UCLASS()
class ARPG_API UEquipSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void NativeConstruct() override;

	// ItemWidget 관리 함수들
	void SetItemWidget(const FInventorySlot& InItemData);
	void ClearItemWidget();

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	void HandleItemEquipped(const FEquippedItem& Item);
public:
	FOnItemEquipped OnItemEquipped;

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* SlotFrame;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equip" , meta = (AllowPrivateAccess = "true"))
	EEquipmentSlot SlotType;

	UPROPERTY(meta = (BindWidget))
	UItemWidget* SlotItemWidget;

public:
	void SetSlotType(EEquipmentSlot NewSlotType) { SlotType = NewSlotType; }
	FORCEINLINE EEquipmentSlot	GetSlotType() const { return SlotType; }
	bool IsEmpty() const { return SlotType == EEquipmentSlot::EES_None; }
	FORCEINLINE UItemWidget* GetItemWidget() const { return SlotItemWidget; }
};
