#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "DiaInventoryPage.generated.h"

class UEquipWidget;
class UMainInventory;

/** Composite menu page that owns the inventory grid and equipment panel. */
UCLASS(Abstract, Blueprintable)
class ARPG_API UDiaInventoryPage : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UEquipWidget* GetEquipmentWidget() const { return EquipmentWidget; }

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory")
	TObjectPtr<UMainInventory> InventoryWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Inventory")
	TObjectPtr<UEquipWidget> EquipmentWidget;
};
