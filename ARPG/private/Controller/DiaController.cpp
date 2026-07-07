// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/DiaController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/EngineBaseTypes.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "DiaComponent/DiaOptionManagerComponent.h"
#include "DiaComponent/DiaSkillManagerComponent.h"

#include "DiaComponent/DiaStatComponent.h"
#include "UI/HUDWidget.h"
#include "UI/Inventory/MainInventory.h"
#include "UI/Inventory/EquipWidget.h"
#include "UI/Inventory/EquipSlot.h"
#include "UI/Skill/SkillPanelWidget.h"

#include "UI/CharacterStatus/StatusWidget.h"
#include "UI/DiaItemDebugWidget.h"
#include "UI/Menu/DiaMenuSystem.h"

#include "GameMode/DungeonGameMode.h"
#include "DiaBaseCharacter.h"
#include "System/ItemSubsystem.h"
#include "Logging/ARPGLogChannels.h"

#include <AbilitySystemBlueprintLibrary.h>

namespace
{
	bool ShouldLogControllerWarning(const UObject* ContextObject, const TCHAR* ContextKey, const double IntervalSeconds = 2.0)
	{
		static TMap<FString, double> LastLogTimes;

		double Now = FPlatformTime::Seconds();
		if (IsValid(ContextObject))
		{
			if (const UWorld* World = ContextObject->GetWorld())
			{
				Now = World->GetTimeSeconds();
			}
		}

		const FString Key = FString::Printf(TEXT("%p-%s"), ContextObject, ContextKey);
		double& LastLogTime = LastLogTimes.FindOrAdd(Key);
		if ((Now - LastLogTime) < IntervalSeconds)
		{
			return false;
		}

		LastLogTime = Now;
		return true;
	}

	TArray<FName> ParseCheatOptionKeys(const FString& OptionKeys)
	{
		TArray<FName> ParsedKeys;
		TArray<FString> Tokens;
		OptionKeys.ParseIntoArray(Tokens, TEXT(","), true);

		for (FString& Token : Tokens)
		{
			Token.TrimStartAndEndInline();
			if (!Token.IsEmpty() && !Token.Equals(TEXT("None"), ESearchCase::IgnoreCase))
			{
				ParsedKeys.Add(FName(*Token));
			}
		}

		return ParsedKeys;
	}
}

ADiaController::ADiaController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

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
		UE_LOG(LogARPG_Inventory, Warning, TEXT("ADiaController: Failed to find HUD widget class"));
	}

	static ConstructorHelpers::FClassFinder<UDiaMenuSystem> MenuSystemClassFinder(TEXT("/Game/UI/Menu/WBP_EscMenu.WBP_EscMenu_C"));
	if (MenuSystemClassFinder.Succeeded())
	{
		MenuSystemClass = MenuSystemClassFinder.Class;
	}
	else
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("ADiaController: Failed to find menu system widget class"));
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
		UE_LOG(LogARPG_Inventory, Warning, TEXT("InventoryWidget is missing from HUDWidget; inventory setup skipped."));
	}

	const bool bHasEquipmentWidget = IsValid(EquipmentWidget);
	if (!bHasEquipmentWidget)
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("EquipmentWidget is missing from HUDWidget; equipment setup skipped."));
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
		UE_LOG(LogARPG_Inventory, Warning, TEXT("DiaInventoryComponent is null"));
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
		UE_LOG(LogARPG_Inventory, Warning, TEXT("DiaEquipmentComponent is null"));
	}

	for (int32 i = 0; i < static_cast<int32>(EEquipmentSlot::EES_Max); ++i)
	{
		UEquipSlot* SlotWidget = EquipmentWidget->GetEquipSlot(static_cast<EEquipmentSlot>(i));
		if (SlotWidget)
		{
			SlotWidget->OnItemEquipped.AddDynamic(this, &ThisClass::OnEquipItemProgress);
		}
	}
	
}

void ADiaController::SetupInputComponent()
{
	Super::SetupInputComponent();

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void ADiaController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (!IsValid(InPawn)) return;

	BindSkillManagerDelegates(InPawn);
}

void ADiaController::BindSkillManagerDelegates(APawn* InPawn)
{
	if (!IsValid(InPawn))
	{
		return;
	}

	UDiaSkillManagerComponent* SkillManagerComponent = InPawn->FindComponentByClass<UDiaSkillManagerComponent>();
	if (!IsValid(SkillManagerComponent))
	{
		return;
	}

	SkillManagerComponent->OnSkillRegistered.RemoveAll(this);
	SkillManagerComponent->OnSkillLevelChanged.RemoveAll(this);
	SkillManagerComponent->OnSkillVariantAdded.RemoveAll(this);

	SkillManagerComponent->OnSkillRegistered.AddUObject(this, &ThisClass::HandleSkillRegistered);
	SkillManagerComponent->OnSkillLevelChanged.AddUObject(this, &ThisClass::HandleSkillLevelChanged);
	SkillManagerComponent->OnSkillVariantAdded.AddUObject(this, &ThisClass::HandleSkillVariantAdded);
}

bool ADiaController::ApplyGoldReward(const FRewardData& RewardData)
{
	if(!IsValid(DiaInventoryComponent) || !IsValid(DiaOptionManagerComponent))
	{
		return false;
	}
	DiaInventoryComponent->AddGoldInventoryWithCheckOption(RewardData.GoldAmount, DiaOptionManagerComponent);
	
	return true;
}

bool ADiaController::ApplyItemReward(const FRewardData& RewardData)
{
	if (!IsValid(DiaInventoryComponent) || !IsValid(DiaOptionManagerComponent))
		return false;
	
	UHUDWidget* HUDWidget = GetHUDWidget();
	if(!IsValid(HUDWidget))
		return false;
	
	UItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UItemSubsystem>();
	if (!ItemSubsystem)
		return false;

	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();
	FInventorySlot NewItemSlot;
	ItemSubsystem->CreateInventoryInstance(NewItemSlot, RewardData.ItemId, RewardData.ItemLevel, true, RewardData.ItemQuantity);
	
	return DiaInventoryComponent->TryAddItem(NewItemSlot, InventoryWidget);
}

bool ADiaController::ApplySkillAddReward(const FRewardData& RewardData)
{
	ADiaBaseCharacter* DiaCharacter = Cast<ADiaBaseCharacter>(GetCharacter());
	if(!IsValid(DiaCharacter))
		return false;

	return DiaCharacter->ApplySkillByID(RewardData.SkillId);
}

bool ADiaController::ApplySkillUpgradeReward(const FRewardData& RewardData)
{
	ADiaBaseCharacter* DiaCharacter = Cast<ADiaBaseCharacter>(GetCharacter());
	if(!IsValid(DiaCharacter))
		return false;

	return DiaCharacter->ApplySkillLevelUpByID(RewardData.SkillId, 1);
}

bool ADiaController::ApplySkillVariantReward(const FRewardData& RewardData)
{
	ADiaBaseCharacter* DiaCharacter = Cast<ADiaBaseCharacter>(GetCharacter());
	if (!IsValid(DiaCharacter))
		return false;

	return DiaCharacter->UnlockSkillVariantByID(RewardData.SkillId, RewardData.VariantId);
}

void ADiaController::HandleSkillRegistered(int32 SkillID, int32 SlotIndex)
{
	ADiaBaseCharacter* DiaCharacter = Cast<ADiaBaseCharacter>(GetCharacter());
	if (!IsValid(DiaCharacter))
		return;
	UDiaSkillManagerComponent* SkillManager = DiaCharacter->FindComponentByClass<UDiaSkillManagerComponent>();
	if (!IsValid(SkillManager))
		return;
	const USkillObject* SkillObj = SkillManager->GetSkillObjectBySkillID(SkillID);
	RegisteSkillPannelWidget(SkillObj);
}

void ADiaController::HandleSkillLevelChanged(int32 SkillID, int32 NewLevel)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		return;
	}

	HUDWidget->UpdateSkillLevel(SkillID, NewLevel);
}

void ADiaController::HandleSkillVariantAdded(int32 SkillID, int32 VariantID)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		return;
	}

	HUDWidget->RegisterSkillVariant(SkillID, VariantID);
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

void ADiaController::ToggleItemDebugUI()
{
	if (IsValid(ItemDebugWidget) && ItemDebugWidget->IsInViewport())
	{
		ItemDebugWidget->RemoveFromParent();
		UE_LOG(LogTemp, Warning, TEXT("Item Debug UI Hidden"));
		return;
	}

	if (!ItemDebugWidgetClass)
	{
		ItemDebugWidgetClass = LoadClass<UDiaItemDebugWidget>(nullptr, TEXT("/Game/UI/Debug/WBP_ItemDebug.WBP_ItemDebug_C"));
	}

	ItemDebugWidget = CreateWidget<UDiaItemDebugWidget>(this, ItemDebugWidgetClass);
	if (!IsValid(ItemDebugWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create ItemDebugWidget"));
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetCharacter());

	ItemDebugWidget->AddToViewport(1000);
	ItemDebugWidget->SetDebugSource(DiaEquipmentComponent, DiaOptionManagerComponent, TargetASC);
	UE_LOG(LogTemp, Warning, TEXT("Item Debug UI Shown"));
}

void ADiaController::SetTarget(ADiaBaseCharacter* NewTarget)
{
	//nullptr이 가능하다.

	TargetMonster = NewTarget;
	OnTargetChanged.Broadcast(TargetMonster);

#if defined(WITH_EDITOR) || UE_BUILD_DEVELOPMENT
	if (TargetMonster)
	{
		//UE_LOG(LogARPG_Inventory, Verbose, TEXT("Target Changed: %s"), *TargetMonster->GetName());
	}
	else
	{
		//UE_LOG(LogARPG_Inventory, Verbose, TEXT("Target Cleared"));
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
		UE_LOG(LogARPG_Inventory, Warning, TEXT("ADiaController::GetHUDWidget - HUDWidgetClass is not set"));
		return nullptr;
	}

	ADiaController* NonConstThis = const_cast<ADiaController*>(this);
	UHUDWidget* NewHUD = CreateWidget<UHUDWidget>(NonConstThis, HUDWidgetClass);
	if (!IsValid(NewHUD))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("ADiaController::GetHUDWidget - Failed to create HUDWidget"));
		return nullptr;
	}

	// AddToViewport()가 NativeConstruct를 유발하고, 그 안에서 GetHUDWidget()이 다시 호출될 수 있음.
	// 재귀 방지를 위해 뷰포트에 추가하기 전에 캐시를 먼저 설정.
	NonConstThis->CachedHUDWidget = NewHUD;
	NewHUD->AddToViewport();

	UE_LOG(LogARPG_Inventory, Display, TEXT("ADiaController::GetHUDWidget - HUDWidget created and cached"));

	return NewHUD;
}

UDiaMenuSystem* ADiaController::GetMenuSystemWidget() const
{
	if (IsValid(MenuSystemWidget))
	{
		return MenuSystemWidget.Get();
	}

	if (!MenuSystemClass)
	{
		return nullptr;
	}

	ADiaController* NonConstThis = const_cast<ADiaController*>(this);
	UDiaMenuSystem* NewWidget = CreateWidget<UDiaMenuSystem>(NonConstThis, MenuSystemClass);
	if (!IsValid(NewWidget))
	{
		return nullptr;
	}

	// AddToViewport()가 NativeConstruct를 유발하고, 그 안에서 GetHUDWidget()이 다시 호출될 수 있음.
	// 재귀 방지를 위해 뷰포트에 추가하기 전에 캐시를 먼저 설정.
	NonConstThis->MenuSystemWidget = NewWidget;
	NewWidget->AddToViewport();


	return NewWidget;
}

//인벤토리에 아이템 더하기 호출.
//외부에서 호출하자.
bool ADiaController::ItemAddedToInventory(const FInventorySlot& Item)
{
	if (!IsValid(DiaInventoryComponent))
	{
		UE_LOG(LogARPG_Inventory, Error, TEXT("DiaInventoryComponent is null"));
		return false;
	}

	ADungeonGameMode* GameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameMode))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("DungeonGameMode is null"));
		return false;
	}

	UHUDWidget* HUDWidget = GetHUDWidget();
	//인벤토리 위젯을 가져와서 넣는다.
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("HUDWidget is null"));
		return false;
	}

    if (Item.ItemInstance.ItemID == "Gold")
    {
        DiaInventoryComponent->AddGoldInventoryWithCheckOption(Item.ItemInstance.Quantity, DiaOptionManagerComponent);
        return true;
    }

	//검증은 tryadditem에서 한다.
	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();

	bool bResult = DiaInventoryComponent->TryAddItem(Item, InventoryWidget);
	if (bResult)
	{
		UE_LOG(LogARPG_Inventory, Verbose, TEXT("Item successfully added to inventory: %s"), *Item.ItemInstance.ItemID.ToString());
	}

	return bResult;
}

void ADiaController::ItemRemoved(const FInventorySlot& Item)
{
	if (!IsValid(DiaInventoryComponent))
	{
		UE_LOG(LogARPG_Inventory, Error, TEXT("DiaInventoryComponent is null"));
		return;
	}

	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("HUDWidget is null"));
		return;
	}

	//Inventory Component에서 제거 

	DiaInventoryComponent->RemoveItem(Item.ItemInstance.InstanceID, HUDWidget->GetInventoryWidget());
}

bool ADiaController::ApplyReward(const FRewardData& RewardData)
{
	switch (RewardData.RewardType)
	{
		case ERewardType::Gold:
			return ApplyGoldReward(RewardData);
		case ERewardType::Item:
			return ApplyItemReward(RewardData);
		case ERewardType::SkillAdd:
			return ApplySkillAddReward(RewardData);
		case ERewardType::SkillUpgrade:
			return ApplySkillUpgradeReward(RewardData);
		case ERewardType::SkillVariant:
			return ApplySkillVariantReward(RewardData);
	}

	return false;
}

void ADiaController::ToggleInventoryVisibility(bool bVisible)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("HUDWidget is null"));
		return;
	}

	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();
	if (!InventoryWidget)
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("InventoryWidget is null"));
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
		UE_LOG(LogARPG_Inventory, Warning, TEXT("HUDWidget is null"));
		return;
	}

	UStatusWidget* StatusWidget = HUDWidget->GetCharacterStatusWidget();
	if (!StatusWidget)
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("StatusWidget is null"));
		return;
	}

	StatusWidget->SetVisibility((bVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void ADiaController::ToggleSkillPanelVisibility(bool bVisible)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("HUDWidget is null"));
		return;
	}
	USkillPanelWidget* SkillPanelWidget = Cast<USkillPanelWidget>(HUDWidget->FindWidgetByName("SkillPanelWidget"));
	if (!SkillPanelWidget)
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("SkillPanelWidget is null"));
		return;
	}
	SkillPanelWidget->SetVisibility((bVisible) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void ADiaController::RegisteSkillOnQuickSlotWidget(int32 SkillID, int32 SlotIndex)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("HUDWidget is null"));
		return;
	}

	HUDWidget->RegisteSkillOnQuickSlotWidget(SkillID, SlotIndex);
}

void ADiaController::RegisteSkillPannelWidget(const USkillObject* SkillData)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("RegisteSkillPannelWidget HUDWidget is null"));
		return;
	}
	if (!IsValid(SkillData))
	{
		UE_LOG(LogARPG_Inventory, Display, TEXT("SkillData is invalid"));
		return;
	}
	HUDWidget->RegisteSkillPannelWidget(SkillData);
}

void ADiaController::RegisteSkillPannelWidget(const TArray<USkillObject*>& SkillDataList)
{
	UHUDWidget* HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("RegisteSkillPannelWidget HUDWidget is null"));
		return;
	}
	if (SkillDataList.Num() == 0)
	{
		UE_LOG(LogARPG_Inventory, Display, TEXT("SkillDataList is empty"));
		return;
	}
	HUDWidget->RegisteSkillPannelWidget(SkillDataList);
}

void ADiaController::CheatDropItem(FName ItemID, int32 Count, int32 Level)
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItem is disabled in shipping builds."));
	return;
#else
	if (ItemID.IsNone())
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItem failed: ItemID is None."));
		return;
	}

	ADungeonGameMode* GameMode = Cast<ADungeonGameMode>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr);
	if (!IsValid(GameMode))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItem failed: DungeonGameMode is null."));
		return;
	}

	UItemSubsystem* ItemSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
	if (!IsValid(ItemSubsystem))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItem failed: ItemSubsystem is null."));
		return;
	}

	AActor* SpawnActor = GetPawn();
	if (!IsValid(SpawnActor))
	{
		SpawnActor = this;
	}

	const FItemBase& ItemData = ItemSubsystem->GetItemData(ItemID);
	if (ItemData.ItemID.IsNone())
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItem failed: invalid ItemID or row name '%s'."), *ItemID.ToString());
		return;
	}

	const int32 SpawnCount = FMath::Clamp(Count, 1, 50);
	const int32 ItemLevel = FMath::Max(Level, 1);
	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		GameMode->SpawnItemAtLocation(SpawnActor, ItemData, ItemLevel);
	}

	UE_LOG(LogARPG_Inventory, Display, TEXT("CheatDropItem: Dropped %d x %s near %s at level %d."),
		SpawnCount,
		*ItemData.ItemID.ToString(),
		*GetNameSafe(SpawnActor),
		ItemLevel);
#endif
}

void ADiaController::CheatGiveItem(FName ItemID, int32 Count, int32 Level)
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItem is disabled in shipping builds."));
	return;
#else
	if (ItemID.IsNone())
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItem failed: ItemID is None."));
		return;
	}

	UItemSubsystem* ItemSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
	if (!IsValid(ItemSubsystem))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItem failed: ItemSubsystem is null."));
		return;
	}

	FName ItemIDCopy = ItemID;
	const int32 GiveCount = FMath::Clamp(Count, 1, 50);
	const int32 ItemLevel = FMath::Max(Level, 1);

	int32 AddedCount = 0;
	for (int32 Index = 0; Index < GiveCount; ++Index)
	{
		FInventorySlot InventoryItem;
		ItemSubsystem->CreateInventoryInstance(InventoryItem, ItemIDCopy, ItemLevel, true);
		if (!InventoryItem.ItemInstance.IsValid())
		{
			UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItem failed: invalid ItemID or row name '%s'."), *ItemID.ToString());
			return;
		}

		if (!ItemAddedToInventory(InventoryItem))
		{
			UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItem stopped: inventory is full or item could not be added. AddedCount=%d"), AddedCount);
			return;
		}

		++AddedCount;
	}

	UE_LOG(LogARPG_Inventory, Display, TEXT("CheatGiveItem: Added %d x %s at level %d."),
		AddedCount,
		*ItemID.ToString(),
		ItemLevel);
#endif
}

void ADiaController::CheatDropItemWithOptions(FName ItemID, const FString& PrefixOptionKeys, const FString& SuffixOptionKeys, int32 Count, int32 Level)
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItemWithOptions is disabled in shipping builds."));
	return;
#else
	if (ItemID.IsNone())
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItemWithOptions failed: ItemID is None."));
		return;
	}

	ADungeonGameMode* GameMode = Cast<ADungeonGameMode>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr);
	if (!IsValid(GameMode))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItemWithOptions failed: DungeonGameMode is null."));
		return;
	}

	UItemSubsystem* ItemSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
	if (!IsValid(ItemSubsystem))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItemWithOptions failed: ItemSubsystem is null."));
		return;
	}

	AActor* SpawnActor = GetPawn();
	if (!IsValid(SpawnActor))
	{
		SpawnActor = this;
	}

	const int32 SpawnCount = FMath::Clamp(Count, 1, 50);
	const int32 ItemLevel = FMath::Max(Level, 1);
	const TArray<FName> PrefixKeys = ParseCheatOptionKeys(PrefixOptionKeys);
	const TArray<FName> SuffixKeys = ParseCheatOptionKeys(SuffixOptionKeys);

	for (int32 Index = 0; Index < SpawnCount; ++Index)
	{
		FInventorySlot InventoryItem;
		FName ItemIDCopy = ItemID;
		ItemSubsystem->CreateInventoryInstanceWithOptions(InventoryItem, ItemIDCopy, ItemLevel, PrefixKeys, SuffixKeys);
		if (!InventoryItem.ItemInstance.IsValid())
		{
			UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatDropItemWithOptions failed: invalid ItemID or row name '%s'."), *ItemID.ToString());
			return;
		}

		GameMode->SpawnInventoryItemAtLocation(SpawnActor, InventoryItem);
	}

	UE_LOG(LogARPG_Inventory, Display, TEXT("CheatDropItemWithOptions: Dropped %d x %s at level %d. Prefix=[%s], Suffix=[%s]"),
		SpawnCount,
		*ItemID.ToString(),
		ItemLevel,
		*PrefixOptionKeys,
		*SuffixOptionKeys);
#endif
}

void ADiaController::CheatGiveItemWithOptions(FName ItemID, const FString& PrefixOptionKeys, const FString& SuffixOptionKeys, int32 Count, int32 Level)
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItemWithOptions is disabled in shipping builds."));
	return;
#else
	if (ItemID.IsNone())
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItemWithOptions failed: ItemID is None."));
		return;
	}

	UItemSubsystem* ItemSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
	if (!IsValid(ItemSubsystem))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItemWithOptions failed: ItemSubsystem is null."));
		return;
	}

	const int32 GiveCount = FMath::Clamp(Count, 1, 50);
	const int32 ItemLevel = FMath::Max(Level, 1);
	const TArray<FName> PrefixKeys = ParseCheatOptionKeys(PrefixOptionKeys);
	const TArray<FName> SuffixKeys = ParseCheatOptionKeys(SuffixOptionKeys);

	int32 AddedCount = 0;
	for (int32 Index = 0; Index < GiveCount; ++Index)
	{
		FInventorySlot InventoryItem;
		FName ItemIDCopy = ItemID;
		ItemSubsystem->CreateInventoryInstanceWithOptions(InventoryItem, ItemIDCopy, ItemLevel, PrefixKeys, SuffixKeys);
		if (!InventoryItem.ItemInstance.IsValid())
		{
			UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItemWithOptions failed: invalid ItemID or row name '%s'."), *ItemID.ToString());
			return;
		}

		if (!ItemAddedToInventory(InventoryItem))
		{
			UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatGiveItemWithOptions stopped: inventory is full or item could not be added. AddedCount=%d"), AddedCount);
			return;
		}

		++AddedCount;
	}

	UE_LOG(LogARPG_Inventory, Display, TEXT("CheatGiveItemWithOptions: Added %d x %s at level %d. Prefix=[%s], Suffix=[%s]"),
		AddedCount,
		*ItemID.ToString(),
		ItemLevel,
		*PrefixOptionKeys,
		*SuffixOptionKeys);
#endif
}

void ADiaController::CheatValidateOptionRolls(FName ItemID, int32 Count, int32 Level)
{
#if UE_BUILD_SHIPPING
	UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatValidateOptionRolls is disabled in shipping builds."));
	return;
#else
	if (ItemID.IsNone())
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatValidateOptionRolls failed: ItemID is None."));
		return;
	}

	UItemSubsystem* ItemSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
	if (!IsValid(ItemSubsystem))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("CheatValidateOptionRolls failed: ItemSubsystem is null."));
		return;
	}

	ItemSubsystem->RunOptionRollStressTest(ItemID, Count, Level);
	if (IsValid(ItemDebugWidget))
	{
		ItemDebugWidget->RefreshDebugText();
	}
#endif
}

void ADiaController::SetMenuSystemVisibility(bool bVisible)
{
	UDiaMenuSystem* MenuWidget = bVisible ? GetMenuSystemWidget() : MenuSystemWidget.Get();
	if (!IsValid(MenuWidget))
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("MenuSystemWidget is null"));
		return;
	}

	MenuWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void ADiaController::ToggleMenuSystemVisibility()
{
	const bool bIsVisible = IsValid(MenuSystemWidget) && MenuSystemWidget->GetVisibility() == ESlateVisibility::Visible;
	SetMenuSystemVisibility(!bIsVisible);
}

ESlateVisibility ADiaController::GetInventoryVisibility() const
{
	const UHUDWidget* const HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		if (ShouldLogControllerWarning(this, TEXT("GetInventoryVisibility_HUDWidgetNull")))
		{
			UE_LOG(LogARPG_Inventory, Warning, TEXT("HUDWidget is null"));
		}
		return ESlateVisibility::Collapsed;
	}

	UMainInventory* InventoryWidget = HUDWidget->GetInventoryWidget();
	if (!InventoryWidget)
	{
		if (ShouldLogControllerWarning(this, TEXT("GetInventoryVisibility_InventoryWidgetNull")))
		{
			UE_LOG(LogARPG_Inventory, Warning, TEXT("InventoryWidget is null"));
		}
		return ESlateVisibility::Collapsed;
	}

	return InventoryWidget->GetVisibility();
}

ESlateVisibility ADiaController::GetWidgetVisibility(const FName& FoundName) const
{
	UHUDWidget* const HUDWidget = GetHUDWidget();
	if (!IsValid(HUDWidget))
	{
		if (ShouldLogControllerWarning(this, TEXT("GetWidgetVisibility_HUDWidgetNull")))
		{
			UE_LOG(LogARPG_Inventory, Warning, TEXT("HUDWidget is null"));
		}
		return ESlateVisibility::Collapsed;
	}
	UUserWidget* FoundWidget = HUDWidget->FindWidgetByName(FoundName);
	if (!IsValid(FoundWidget))
	{
		if (ShouldLogControllerWarning(this, *FString::Printf(TEXT("GetWidgetVisibility_%s_NotFound"), *FoundName.ToString())))
		{
			UE_LOG(LogARPG_Inventory, Warning, TEXT("Widget with name '%s' not found in HUDWidget"), *FoundName.ToString());
		}
		return ESlateVisibility::Collapsed;
	}
	return FoundWidget->GetVisibility();
}
