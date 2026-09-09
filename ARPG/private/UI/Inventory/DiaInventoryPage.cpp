#include "UI/Inventory/DiaInventoryPage.h"

#include "Controller/DiaController.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "Logging/ARPGLogChannels.h"
#include "UI/Inventory/EquipSlot.h"
#include "UI/Inventory/EquipWidget.h"
#include "UI/Inventory/MainInventory.h"

void UDiaInventoryPage::NativeConstruct()
{
	Super::NativeConstruct();

	ADiaController* DiaController = Cast<ADiaController>(GetOwningPlayer());
	if (!IsValid(DiaController) || !IsValid(InventoryWidget) || !IsValid(EquipmentWidget))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("DiaInventoryPage initialization failed: controller or bound widgets are invalid."));
		return;
	}

	UDiaInventoryComponent* InventoryComponent = DiaController->GetInventoryComponent();
	UDiaEquipmentComponent* EquipmentComponent = DiaController->GetEquipmentComponent();
	if (!IsValid(InventoryComponent) || !IsValid(EquipmentComponent))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("DiaInventoryPage initialization failed: inventory components are invalid."));
		return;
	}

	InventoryWidget->SetInventoryComponent(InventoryComponent);
	InventoryWidget->SetEquipmentComponent(EquipmentComponent);
	InventoryWidget->InitializeInventory();

	EquipmentWidget->SetDiaController(DiaController);
	EquipmentWidget->SetInventoryComponent(InventoryComponent);
	EquipmentWidget->SetEquipmentComponent(EquipmentComponent);
	EquipmentWidget->InitializeEquipment();

	for (int32 Index = 0; Index < static_cast<int32>(EEquipmentSlot::EES_Max); ++Index)
	{
		if (UEquipSlot* SlotWidget = EquipmentWidget->GetEquipSlot(static_cast<EEquipmentSlot>(Index)))
		{
			SlotWidget->OnItemEquipped.AddUniqueDynamic(DiaController, &ADiaController::OnEquipItemProgress);
		}
	}
}
