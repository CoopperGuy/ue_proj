// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DiaSaveSubsystem.h"
#include "System/DiaSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/DiaController.h"
#include "DiaBaseCharacter.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "GAS/DiaAttributeSet.h"
#include "System/CharacterManager.h"

#include "AbilitySystemBlueprintLibrary.h"

#include "Logging/ARPGLogChannels.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

namespace
{
	constexpr int32 MaxPendingLoadApplyAttempts = 10;

	bool IsPIEPrefixedMapName(const FString& LevelName)
	{
		return LevelName.StartsWith(TEXT("UEDPIE_"));
	}
}


//일단 동기 세이브,로드로 진행
void UDiaSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnPostLoadMapWithWorld);
}

void UDiaSaveSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	PendingLoadedGame = nullptr;
	PendingLoadApplyAttempts = 0;

	Super::Deinitialize();
}

bool UDiaSaveSubsystem::SaveGame(const FString& SlotName)
{
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame failed: SlotName is empty."));
		return false;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame failed: World is invalid. Slot=%s"), *SlotName);
		return false;
	}

	ADiaController* Controller = Cast<ADiaController>(UGameplayStatics::GetPlayerController(World, 0));
	if (!IsValid(Controller))
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame failed: DiaController not found. Slot=%s"), *SlotName);
		return false;
	}

	const ADiaBaseCharacter* PlayerCharacter = Cast<ADiaBaseCharacter>(Controller->GetPawn());
	if (!IsValid(PlayerCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame failed: player pawn not found. Slot=%s"), *SlotName);
		return false;
	}

	if (!IsValid(Controller->GetComponentByClass<UDiaInventoryComponent>()) ||
		!IsValid(Controller->GetComponentByClass<UDiaEquipmentComponent>()))
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame failed: inventory or equipment component missing. Slot=%s"), *SlotName);
		return false;
	}

	UDiaSaveGame* SaveGameInstance = Cast<UDiaSaveGame>(UGameplayStatics::CreateSaveGameObject(UDiaSaveGame::StaticClass()));
	if (!IsValid(SaveGameInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGame failed: could not create save object. Slot=%s"), *SlotName);
		return false;
	}

	SaveGameInstance->PlayerName = TEXT("PlayerOne");
	SaveGameInstance->SaveLevelName = UGameplayStatics::GetCurrentLevelName(World, true);
	FillSaveDataByGame(SaveGameInstance, Controller);

	return UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
}

bool UDiaSaveSubsystem::LoadGame(const FString& SlotName)
{

	if (UDiaSaveGame* LoadedGame = Cast<UDiaSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
	{
		const FString SaveLevelName = NormalizeSavedLevelName(LoadedGame->SaveLevelName);
		if (SaveLevelName.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadGame failed: SaveLevelName is empty. Slot=%s"), *SlotName);
			return false;
		}

		PendingLoadedGame = LoadedGame;
		PendingLoadApplyAttempts = 0;
		UGameplayStatics::OpenLevel(GetWorld(), FName(*SaveLevelName));

		UE_LOG(LogTemp, Warning, TEXT("LOAD REQUESTED: %s / Level=%s"), *LoadedGame->PlayerName, *SaveLevelName);
		return true;
	}

	return false;
}

bool UDiaSaveSubsystem::DoesSaveExist(const FString& SlotName) const
{
	return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

void UDiaSaveSubsystem::DeleteSave(const FString& SlotName)
{
	if (SlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("DeleteSave failed: SlotName is empty."));
		return;
	}

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return;
	}

	if (!UGameplayStatics::DeleteGameInSlot(SlotName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("DeleteSave failed: could not delete slot. Slot=%s"), *SlotName);
	}
}

void UDiaSaveSubsystem::OnPostLoadMapWithWorld(UWorld* InWorld)
{
	if (!IsValid(PendingLoadedGame) || !IsValid(InWorld))
	{
		return;
	}

	FTimerDelegate ApplySaveDelegate;
	ApplySaveDelegate.BindUObject(this, &ThisClass::ApplyPendingSaveData, InWorld);
	InWorld->GetTimerManager().SetTimerForNextTick(ApplySaveDelegate);
}

void UDiaSaveSubsystem::ApplyPendingSaveData(UWorld* InWorld)
{
	if (!IsValid(PendingLoadedGame) || !IsValid(InWorld))
	{
		return;
	}

	ADiaController* Controller = Cast<ADiaController>(UGameplayStatics::GetPlayerController(InWorld, 0));
	ADiaBaseCharacter* PlayerCharacter = IsValid(Controller) ? Cast<ADiaBaseCharacter>(Controller->GetPawn()) : nullptr;
	if (!IsValid(Controller) || !IsValid(PlayerCharacter))
	{
		++PendingLoadApplyAttempts;
		if (PendingLoadApplyAttempts <= MaxPendingLoadApplyAttempts)
		{
			FTimerDelegate RetryDelegate;
			RetryDelegate.BindUObject(this, &ThisClass::ApplyPendingSaveData, InWorld);
			InWorld->GetTimerManager().SetTimerForNextTick(RetryDelegate);
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("LoadGame failed: DiaController or player pawn not found after level load."));
		PendingLoadedGame = nullptr;
		PendingLoadApplyAttempts = 0;
		return;
	}

	ApplySaveDataToGame(PendingLoadedGame, Controller);
	UE_LOG(LogTemp, Warning, TEXT("LOADED: %s"), *PendingLoadedGame->PlayerName);
	PendingLoadedGame = nullptr;
	PendingLoadApplyAttempts = 0;
}

FString UDiaSaveSubsystem::NormalizeSavedLevelName(const FString& LevelName) const
{
	FString NormalizedName = LevelName;

	int32 FirstUnderscoreIndex = INDEX_NONE;
	if (IsPIEPrefixedMapName(NormalizedName) && NormalizedName.FindChar(TEXT('_'), FirstUnderscoreIndex))
	{
		const int32 SecondUnderscoreIndex = NormalizedName.Find(TEXT("_"), ESearchCase::CaseSensitive, ESearchDir::FromStart, FirstUnderscoreIndex + 1);
		if (SecondUnderscoreIndex != INDEX_NONE)
		{
			NormalizedName.RightChopInline(SecondUnderscoreIndex + 1);
		}
	}

	return NormalizedName;
}

void UDiaSaveSubsystem::FillSaveDataByGame(UDiaSaveGame* SaveGameInstance, const ADiaController* Controller)
{
	if (!IsValid(SaveGameInstance) || !IsValid(Controller))
	{
		return;
	}

	const ADiaBaseCharacter* PlayerCharacter = Cast<ADiaBaseCharacter>(Controller->GetPawn());
	if(IsValid(PlayerCharacter))
	{
		UDiaInventoryComponent* InvenComp = Controller->GetComponentByClass<UDiaInventoryComponent>();
		UDiaEquipmentComponent* EquipComp = Controller->GetComponentByClass<UDiaEquipmentComponent>();
		if (IsValid(InvenComp) && IsValid(EquipComp))
		{
			InvenComp->SaveInventoryToSaveGame(SaveGameInstance);
			EquipComp->SaveEquipmentToSaveGame(SaveGameInstance);
		}

		if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
		{
			if (const UDiaAttributeSet* Attr = Cast<UDiaAttributeSet>(ASC->GetAttributeSet(UDiaAttributeSet::StaticClass())))
			{
				SaveGameInstance->PlayerLevel = FMath::Max(1, FMath::RoundToInt(Attr->GetLevel()));
			}
		}

		SaveGameInstance->PlayerLocation = PlayerCharacter->GetActorLocation();
	}
}
void UDiaSaveSubsystem::ApplySaveDataToGame(const UDiaSaveGame* SaveGameInstance, ADiaController* Controller)
{
	if(!IsValid(SaveGameInstance) || !IsValid(Controller))
	{
		return;
	}

	ADiaBaseCharacter* PlayerCharacter = Cast<ADiaBaseCharacter>(Controller->GetPawn());
	if (IsValid(PlayerCharacter))
	{
		UDiaInventoryComponent* InvenComp = Controller->GetComponentByClass<UDiaInventoryComponent>();
		UDiaEquipmentComponent* EquipComp = Controller->GetComponentByClass<UDiaEquipmentComponent>();
		if (IsValid(InvenComp) && IsValid(EquipComp))
		{
			InvenComp->LoadInventoryFromSaveGame(SaveGameInstance);
			EquipComp->LoadEquipmentFromSaveGame(SaveGameInstance);
		}

		if (UDiaAttributeSet* Attr = PlayerCharacter->GetAttributeSet())
		{
			const int32 LoadedLevel = FMath::Max(1, SaveGameInstance->PlayerLevel);
			if (UCharacterManager* CharacterManager = GetGameInstance()->GetSubsystem<UCharacterManager>())
			{
				Attr->InitializeCharacterAttributes(CharacterManager->DefaultCharacterID, LoadedLevel);
			}
			else
			{
				Attr->SetLevel(LoadedLevel);
			}
		}

		PlayerCharacter->SetActorLocation(SaveGameInstance->PlayerLocation);
	}
}
