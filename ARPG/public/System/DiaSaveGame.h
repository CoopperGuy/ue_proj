// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Types/ItemBase.h"
#include "System/DiaMapGeneratorSubsystem.h"
#include "Types/DiaRoomRewardState.h"
#include "DiaSaveGame.generated.h"

USTRUCT()
struct FDiaSavedSkill
{
    GENERATED_BODY()
    UPROPERTY() int32 SkillID = 0;
    UPROPERTY() int32 Level = 1;
    UPROPERTY() TSet<int32> OwnedVariants;
    UPROPERTY() TSet<int32> AppliedVariants;
};

/**
 * 
 */
UCLASS()
class ARPG_API UDiaSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
    // Zero is intentionally the legacy default when old slots omit this field.
    bool IsFloorSnapshotValid() const;
    UPROPERTY() bool bHasDungeonFloor = false;
    UPROPERTY() int32 SaveVersion = 0;
    UPROPERTY() int32 Gold = 0;
    UPROPERTY() float Health = 0.f;
    UPROPERTY() float Mana = 0.f;
    UPROPERTY() float Experience = 0.f;
    UPROPERTY() TArray<FDiaSavedSkill> Skills;
    UPROPERTY() TArray<int32> QuickSlots;
    UPROPERTY() FIntPoint FloorSize = FIntPoint::ZeroValue;
    UPROPERTY() TArray<FDiaRoomData> RoomLayout;
    UPROPERTY() FDiaRoomData StartRoom;
    UPROPERTY() TSet<FGuid> ClearedRooms;
    UPROPERTY() TMap<FGuid, FDiaRoomRewardState> RoomRewards;
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
