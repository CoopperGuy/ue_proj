// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/ItemBase.h"
#include "EquipWidget.generated.h"

class UEquipSlot;
class ADiaController;
class UDiaInventoryComponent;
class UDiaEquipmentComponent;

/**
 * 
 */
UCLASS()
class ARPG_API UEquipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void NativeConstruct() override;
	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void EquipItemToSlot(EEquipmentSlot SlotType, const FEquippedItem& Item);
	UFUNCTION()
	void UnequipItemFromSlot(EEquipmentSlot SlotType);

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:
	UPROPERTY(meta = (BindWidget))
	UEquipSlot* ArmorSlot;

	UPROPERTY(meta = (BindWidget))
	UEquipSlot* LeftWeaponSlot;

	UPROPERTY(meta = (BindWidget))
	UEquipSlot* ShooseSlot;

	UPROPERTY(meta = (BindWidget))
	UEquipSlot* HandsSlot;

	UPROPERTY(meta = (BindWidget))
	UEquipSlot* HelmetSlot;

	TWeakObjectPtr<UDiaInventoryComponent> InventoryComponent;
	TWeakObjectPtr<UDiaEquipmentComponent> EquipementComponent;
	TWeakObjectPtr<ADiaController> DiaControllerRef;
public:
	UEquipSlot* GetEquipSlot(EEquipmentSlot SlotType) const
	{
		switch (SlotType)
		{
		case EEquipmentSlot::EES_Chest: return ArmorSlot;
		case EEquipmentSlot::EES_Weapon: return LeftWeaponSlot;
		case EEquipmentSlot::EES_Feet: return ShooseSlot;
		case EEquipmentSlot::EES_Hands: return HandsSlot;
		case EEquipmentSlot::EES_Head: return HelmetSlot;
		default: return nullptr;
		}
	}
	void SetInventoryComponent(UDiaInventoryComponent* InComponent);
	void SetEquipmentComponent(UDiaEquipmentComponent* InComponent);
	void SetDiaController(ADiaController* InController);
};
