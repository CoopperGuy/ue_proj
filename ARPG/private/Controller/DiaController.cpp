// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/DiaController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/EngineBaseTypes.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "DiaComponent/DiaOptionManagerComponent.h"

#include "DiaComponent/DiaStatComponent.h"
#include "UI/HUDWidget.h"
#include "UI/System/DiaPrimaryLayout.h"

#include "UI/DiaItemDebugWidget.h"
#include "UI/Menu/DiaMenuSystem.h"

#include "GameMode/DungeonGameMode.h"
#include "DiaBaseCharacter.h"
#include "Character/DiaCharacter.h"

#include "System/ItemSubsystem.h"
#include "System/GameViewPort/DiaCustomGameViewPort.h"
#include "Logging/ARPGLogChannels.h"

#include "Interface/DiaInteractable.h"

#include "GAS/DiaGameplayTags.h"

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

	//치트용 함수
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

	UDiaCustomGameViewPort* DiaCustomGameViewPort = GetWorld() ? Cast<UDiaCustomGameViewPort>(GetWorld()->GetGameViewport()) : nullptr;
	if (!IsValid(DiaCustomGameViewPort) || !IsValid(DiaCustomGameViewPort->CreatePrimaryLayout(this)))
	{
		return;
	}

	if (IsValid(DiaInventoryComponent))
	{
		if (!DiaInventoryComponent->IsRegistered())
		{
			DiaInventoryComponent->RegisterComponent();
		}
	}
	else
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("DiaInventoryComponent is null"));
	}

	if (IsValid(DiaEquipmentComponent))
	{
		if (!DiaEquipmentComponent->IsRegistered())
		{
			DiaEquipmentComponent->RegisterComponent();
		}
	}
	else
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("DiaEquipmentComponent is null"));
	}

	if (IsValid(DiaEquipmentComponent) && IsValid(DiaInventoryComponent) && IsValid(DiaOptionManagerComponent))
	{
		DiaEquipmentComponent->SetInventoryComponent(DiaInventoryComponent);
		DiaEquipmentComponent->SetOptionManagerComponent(DiaOptionManagerComponent);
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
	
	UItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UItemSubsystem>();
	if (!ItemSubsystem)
		return false;

	FInventorySlot NewItemSlot;
	ItemSubsystem->CreateInventoryInstance(NewItemSlot, RewardData.ItemId, RewardData.ItemLevel, true, RewardData.ItemQuantity);
	
	return DiaInventoryComponent->TryAddItem(NewItemSlot);
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

void ADiaController::NotifyPlayerASCReady(ADiaBaseCharacter* PlayerCharacter, UAbilitySystemComponent* AbilitySystem)
{
	if (!IsValid(PlayerCharacter) || !IsValid(AbilitySystem) || GetPawn() != PlayerCharacter)
	{
		return;
	}

	OnPlayerASCReady.Broadcast(PlayerCharacter, AbilitySystem);
}

UHUDWidget* ADiaController::GetHUDWidget() const
{
	UDiaCustomGameViewPort* DiaCumstomGameViewPort = GetWorld() ? Cast<UDiaCustomGameViewPort>(GetWorld()->GetGameViewport()) : nullptr;
	if (!IsValid(DiaCumstomGameViewPort))
	{
		return nullptr;
	}
	UDiaPrimaryLayout* DiaPrimaryLayout = DiaCumstomGameViewPort->GetDiaPrimaryLayout();
	if(!IsValid(DiaPrimaryLayout))
	{
		return nullptr;
	}

	return DiaPrimaryLayout->GetHUDWidget();
}

void ADiaController::ApplyInputRouting(EDiaInputRouting NewMode)
{
	CurrentUIMode = NewMode;

	ADiaCharacter* DiaCharacter = Cast<ADiaCharacter>(GetCharacter());
	if (!IsValid(DiaCharacter))
		return;

	switch (CurrentUIMode)
	{
	case EDiaInputRouting::GameOnly:
		DiaCharacter->HandleGameplayInputRouting(this, true);
		DiaCharacter->HandlePersistentInputRouting(this, false);
		break;
	case EDiaInputRouting::UIOnly:
		DiaCharacter->HandleGameplayInputRouting(this, false);
		DiaCharacter->HandlePersistentInputRouting(this, true);
		break;
	case EDiaInputRouting::GameAndUI:
		DiaCharacter->HandleGameplayInputRouting(this, true);
		DiaCharacter->HandlePersistentInputRouting(this, true);
		break;
	default:
		DiaCharacter->HandleGameplayInputRouting(this, true);
		DiaCharacter->HandlePersistentInputRouting(this, true);
		break;
	}
}


bool ADiaController::IsSkillInputBlocked() const
{
	if (CurrentUIMode == EDiaInputRouting::UIOnly)
		return true;

	return false;
}

bool ADiaController::IsUIInputBlocked() const
{
	if (CurrentUIMode == EDiaInputRouting::GameOnly)
		return true;

	return false;
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


	//인벤토리 위젯을 가져와서 넣는다.
	if (Item.ItemInstance.ItemID == "Gold")
    {
        DiaInventoryComponent->AddGoldInventoryWithCheckOption(Item.ItemInstance.Quantity, DiaOptionManagerComponent);
        return true;
    }

	//검증은 tryadditem에서 한다.
	bool bResult = DiaInventoryComponent->TryAddItem(Item);
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

	//Inventory Component에서 제거 

	DiaInventoryComponent->RemoveItem(Item.ItemInstance.InstanceID);
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

bool ADiaController::ToggleMenuPage(const FGameplayTag& MenuPageTag)
{
	UDiaCustomGameViewPort* DiaCustomGameViewPort = GetWorld() ? Cast<UDiaCustomGameViewPort>(GetWorld()->GetGameViewport()) : nullptr;
	if (!IsValid(DiaCustomGameViewPort))
	{
		return false;
	}

	UDiaPrimaryLayout* DiaPrimaryLayout = DiaCustomGameViewPort->GetDiaPrimaryLayout();
	if (!IsValid(DiaPrimaryLayout))
	{
		return false;
	}

	return DiaPrimaryLayout->ToggleMenu(MenuPageTag);
}

bool ADiaController::ToggleInventoryVisibility()
{
	return ToggleMenuPage(FDiaGameplayTags::Get().UI_MenuPage_Inventory);
}

bool ADiaController::ToggleCharacterStatusVisibility()
{
	return ToggleMenuPage(FDiaGameplayTags::Get().UI_MenuPage_Character);
}

bool ADiaController::ToggleSkillPanelVisibility()
{
	return ToggleMenuPage(FDiaGameplayTags::Get().UI_MenuPage_Skill);
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

bool ADiaController::HandlePrimaryClick()
{
	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

	for (AActor* Actor = HitResult.GetActor(); Actor; Actor = Actor->GetAttachParentActor())
	{
		if (IDiaInteractable* Interactable = Cast<IDiaInteractable>(Actor); Interactable && Interactable->IsInteractable())
		{
			Interactable->OnInteract(this);
			return true;
		}
	}

	return false;
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
	UDiaCustomGameViewPort* DiaCustomGameViewPort = GetWorld() ? Cast<UDiaCustomGameViewPort>(GetWorld()->GetGameViewport()) : nullptr;
	if (!IsValid(DiaCustomGameViewPort))
	{
		return ;
	}

	UDiaPrimaryLayout* DiaPrimaryLayout = DiaCustomGameViewPort->GetDiaPrimaryLayout();
	if (!IsValid(DiaPrimaryLayout))
	{
		return ;
	}

	if (DiaPrimaryLayout->IsMainMenuOpen())
	{
		DiaPrimaryLayout->CloseMenu();
	}
	else
	{
		const bool bIsVisible = IsValid(MenuSystemWidget) && MenuSystemWidget->GetVisibility() == ESlateVisibility::Visible;
		SetMenuSystemVisibility(!bIsVisible);
	}
}
