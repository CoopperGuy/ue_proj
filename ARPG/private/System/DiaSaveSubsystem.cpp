// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DiaSaveSubsystem.h"
#include "System/DiaSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/DiaController.h"
#include "DiaBaseCharacter.h"

#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "GAS/DiaAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"

#include "Logging/ARPGLogChannels.h"


//일단 동기 세이브,로드로 진행
bool UDiaSaveSubsystem::SaveGame(const FString& SlotName)
{
	UDiaSaveGame* SaveGameInstance = Cast<UDiaSaveGame>(UGameplayStatics::CreateSaveGameObject(UDiaSaveGame::StaticClass()));
	//Create Save Data Safely
	if (IsValid(SaveGameInstance))
	{
		// Set data on the savegame object.
		SaveGameInstance->PlayerName = TEXT("PlayerOne");

		ADiaController* Controller = Cast<ADiaController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
		FillSaveDataByGame(SaveGameInstance, Controller);

		// Save the data immediately.
		if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0))
		{
			// Save succeeded.
		}
		else
		{
			return false;
		}
	}
	return true;
}


bool UDiaSaveSubsystem::LoadGame(const FString& SlotName)
{
	// Retrieve and cast the USaveGame object to UMySaveGame.
	if (UDiaSaveGame* LoadedGame = Cast<UDiaSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
	{
		// The operation was successful, so LoadedGame now contains the data we saved earlier.
		UE_LOG(LogTemp, Warning, TEXT("LOADED: %s"), *LoadedGame->PlayerName);
	}
	else
	{
		return false;
	}

	return true;
}

bool UDiaSaveSubsystem::DoesSaveExist(const FString& SlotName) const
{
	return false;
}

void UDiaSaveSubsystem::DeleteSave(const FString& SlotName)
{
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
}
