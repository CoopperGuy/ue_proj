// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Types/ItemBase.h"
#include "DiaSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	TArray<FInventorySlot> InventorySlots;

	UPROPERTY()
	TMap<EEquipmentSlot, FEquippedItem> EquippedItems;

	UPROPERTY()
	int32 PlayerLevel = 1;

	UPROPERTY()
	FVector PlayerLocation;

	UPROPERTY()
	FString SaveLevelName;
		
};
