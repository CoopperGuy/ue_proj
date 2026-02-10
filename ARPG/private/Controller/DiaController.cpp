// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/DiaController.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "DiaComponent/DiaOptionManagerComponent.h"

#include "DiaComponent/DiaStatComponent.h"
#include "UI/HUDWidget.h"
#include "UI/Inventory/MainInventory.h"
#include "UI/Inventory/EquipWidget.h"
#include "UI/Inventory/EquipSlot.h"
#include "UI/Skill/SkillPanelWidget.h"

#include "UI/CharacterStatus/StatusWidget.h"

#include "GameMode/DungeonGameMode.h"
#include "DiaBaseCharacter.h"

ADiaController::ADiaController()
{
	bShowMouseCursor = true;
	DiaInventoryComponent = CreateDefaultSubobject<UDiaInventoryComponent>(TEXT("InventoryComponent"));
	DiaEquipmentComponent = CreateDefaultSubobject<UDiaEquipmentComponent>(TEXT("EquipmentComponent"));
	DiaOptionManagerComponent = CreateDefaultSubobject<UDiaOptionManagerComponent>(TEXT("OptionManagerComponent"));

	// HUD 위젯 클래스 C++에서 설정
	// Blueprint 생성 클래스는 이름 뒤에 _C 접미사가 붙음
	static ConstructorHelpers::FClassFinder<UHUDWidget> HUDWidgetClassFinder(TEXT("/Game/UI/HUD/WBP_HUDWidget.WBP_HUDWidget_C"));
	if (HUDWidgetClassFinder.Succeeded())
	{
		HUDWidgetClass = HUDWidgetClassFinder.Class;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ADiaController: Failed to find HUD widget class"));
	}
}

void ADiaController::BeginPlay()
{
	Super::BeginPlay();
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		return;
	}
	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();
	UEquipWidget* EquipmentWidget = Cast<UEquipWidget>(HUDWidget->FindWidgetByName("EquipmentWidget"));

	const bool bHasInventoryWidget = IsValid(InventoryWidget);
	if (!bHasInventoryWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget is missing from HUDWidget; inventory setup skipped."));
	}

	const bool bHasEquipmentWidget = IsValid(EquipmentWidget);
	if (!bHasEquipmentWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipmentWidget is missing from HUDWidget; equipment setup skipped."));
	}

	if (!bHasInventoryWidget && !bHasEquipmentWidget)
	{
		return;
	}

	if (IsValid(DiaInventoryComponent))
	{
		DiaInventoryComponent->RegisterComponent();
		if (bHasInventoryWidget)
		{
			InventoryWidget->SetInventoryComponent(DiaInventoryComponent);
			InventoryWidget->InitializeInventory();
		}

		if (bHasEquipmentWidget)
		{
			EquipmentWidget->SetInventoryComponent(DiaInventoryComponent);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaInventoryComponent is null"));
	}

	if (IsValid(DiaEquipmentComponent))
	{
		DiaEquipmentComponent->RegisterComponent();
		if (bHasInventoryWidget)
		{
			InventoryWidget->SetEquipmentComponent(DiaEquipmentComponent);
		}

		if (bHasEquipmentWidget)
		{
			EquipmentWidget->SetDiaController(this);
			EquipmentWidget->SetEquipmentComponent(DiaEquipmentComponent);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaEquipmentComponent is null"));
	}

	for (int32 i = 0; i < static_cast<int32>(EEquipmentSlot::EES_Max); ++i)
	{
		UEquipSlot* SlotWidget = EquipmentWidget->GetEquipSlot(static_cast<EEquipmentSlot>(i));
		if (SlotWidget)
		{
			SlotWidget->OnItemEquipped.AddDynamic(this, &ThisClass::OnEquipItemProgress);
		}
	}
	//
}

void ADiaController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void ADiaController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (!IsValid(InPawn)) return;
}

void ADiaController::OnEquipItemProgress(const FEquippedItem& Item, EEquipmentSlot SlotType)
{
	if (!IsValid(DiaEquipmentComponent) || !IsValid(DiaOptionManagerComponent))
	{
		return;
	}

	DiaEquipmentComponent->EquipItem(Item, SlotType);
	DiaOptionManagerComponent->ApplyEquipmentStats(Item, SlotType);

	for (const auto& option : Item.ItemInstance.PrefixOptions)
	{
		DiaOptionManagerComponent->AddOption(option);
	}
	for (const auto& option : Item.ItemInstance.SuffixOptions)
	{
		DiaOptionManagerComponent->AddOption(option);
	}

	DiaOptionManagerComponent->ApplyEquipmentSlotOption(Item);
}

void ADiaController::OnUnequipItemProgress(EEquipmentSlot SlotType)
{
	if (!IsValid(DiaEquipmentComponent) || !IsValid(DiaOptionManagerComponent))
	{
		return;
	}
	const FEquippedItem* Item = DiaEquipmentComponent->GetEquippedItem(SlotType);
	if (Item == nullptr)
	{
		return;
	}

	DiaOptionManagerComponent->RemoveEqipmentStats(*Item, SlotType);
	
	for (const auto& option : Item->ItemInstance.PrefixOptions)
	{
		DiaOptionManagerComponent->RemoveOption(option);
	}
	for (const auto& option : Item->ItemInstance.SuffixOptions)
	{
		DiaOptionManagerComponent->RemoveOption(option);
	}

	DiaEquipmentComponent->UnEquipItemFinish(SlotType);
}

void ADiaController::SetTarget(ADiaBaseCharacter* NewTarget)
{
	//nullptr이 가능하다.

	TargetMonster = NewTarget;
	OnTargetChanged.Broadcast(TargetMonster);

#if defined(WITH_EDITOR) || UE_BUILD_DEVELOPMENT
	if (TargetMonster)
	{
		//UE_LOG(LogTemp, Log, TEXT("Target Changed: %s"), *TargetMonster->GetName());
	}
	else
	{
		//UE_LOG(LogTemp, Log, TEXT("Target Cleared"));
	}
#endif
}

UHUDWidget* ADiaController::GetHUDWidget() const
{
	if (CachedHUDWidget.IsValid())
	{
		return CachedHUDWidget.Get();
	}

	if (!HUDWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ADiaController::GetHUDWidget - HUDWidgetClass is not set"));
		return nullptr;
	}

	ADiaController* NonConstThis = const_cast<ADiaController*>(this);
	UHUDWidget* NewHUD = CreateWidget<UHUDWidget>(NonConstThis, HUDWidgetClass);
	if (!IsValid(NewHUD))
	{
		UE_LOG(LogTemp, Warning, TEXT("ADiaController::GetHUDWidget - Failed to create HUDWidget"));
		return nullptr;
	}

	// AddToViewport()가 NativeConstruct를 유발하고, 그 안에서 GetHUDWidget()이 다시 호출될 수 있음.
	// 재귀 방지를 위해 뷰포트에 추가하기 전에 캐시를 먼저 설정.
	NonConstThis->CachedHUDWidget = NewHUD;
	NewHUD->AddToViewport();

	UE_LOG(LogTemp, Log, TEXT("ADiaController::GetHUDWidget - HUDWidget created and cached"));

	return NewHUD;
}

//인벤토리에 아이템 더하기 호출.
//외부에서 호출하자.
bool ADiaController::ItemAddedToInventory(const FInventorySlot& Item)
{
	if (!IsValid(DiaInventoryComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("DiaInventoryComponent is null"));
		return false;
	}

	ADungeonGameMode* GameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("DungeonGameMode is null"));
		return false;
	}

	UHUDWidget* HUDWidget = GetHUDWidget();
	//인벤토리 위젯을 가져와서 넣는다.
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return false;
	}

	//검증은 tryadditem에서 한다.
	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();

	bool bResult = DiaInventoryComponent->TryAddItem(Item, InventoryWidget);
	if (bResult)
	{
		UE_LOG(LogTemp, Log, TEXT("Item successfully added to inventory: %s"), *Item.ItemInstance.BaseItem.ItemID.ToString());
	}

	return bResult;
}

void ADiaController::ItemRemoved(const FInventorySlot& Item)
{
	if (!IsValid(DiaInventoryComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("DiaInventoryComponent is null"));
		return;
	}

	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return;
	}

	//Inventory Component에서 제거 

	DiaInventoryComponent->RemoveItem(Item.ItemInstance.InstanceID, HUDWidget->GetInventoryWidget());
}

void ADiaController::ToggleInventoryVisibility(bool bVisible)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return;
	}

	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();
	if (!InventoryWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget is null"));
		return;
	}
	UEquipWidget* EquipmentWidget = HUDWidget->GetEquipmentWidget();

	InventoryWidget->SetVisibility((bVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	EquipmentWidget->SetVisibility((bVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void ADiaController::ToggleChracterStatusVisibility(bool bVisible)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return;
	}

	UStatusWidget* StatusWidget = HUDWidget->GetCharacterStatusWidget();
	if (!StatusWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("StatusWidget is null"));
		return;
	}

	StatusWidget->SetVisibility((bVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void ADiaController::ToggleSkillPanelVisibility(bool bVisible)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return;
	}
	USkillPanelWidget* SkillPanelWidget = Cast<USkillPanelWidget>(HUDWidget->FindWidgetByName("SkillPanelWidget"));
	if (!SkillPanelWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillPanelWidget is null"));
		return;
	}
	SkillPanelWidget->SetVisibility((bVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void ADiaController::RegisteSkillOnQuickSlotWidget(int32 SkillID, int32 SlotIndex)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return;
	}

	HUDWidget->RegisteSkillOnQuickSlotWidget(SkillID, SlotIndex);
}

void ADiaController::RegisteSkillPannelWidget(const TArray<USkillObject*>& SkillDataList)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisteSkillPannelWidget HUDWidget is null"));
		return;
	}
	if (SkillDataList.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillDataList is empty"));
		return;
	}
	HUDWidget->RegisteSkillPannelWidget(SkillDataList);
}

ESlateVisibility ADiaController::GetInventoryVisibility() const
{
	const UHUDWidget* const HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return ESlateVisibility::Collapsed;
	}

	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();
	if (!InventoryWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget is null"));
		return ESlateVisibility::Collapsed;
	}

	return InventoryWidget->GetVisibility();
}

ESlateVisibility ADiaController::GetWidgetVisibility(const FName& FoundName) const
{
	UHUDWidget* const HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidget is null"));
		return ESlateVisibility::Collapsed;
	}
	UUserWidget* FoundWidget = HUDWidget->FindWidgetByName(FoundName);
	if (!IsValid(FoundWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("Widget with name '%s' not found in HUDWidget"), *FoundName.ToString());
		return ESlateVisibility::Collapsed;
	}
	return FoundWidget->GetVisibility();
}


