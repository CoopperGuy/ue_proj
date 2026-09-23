// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DiaSaveSubsystem.h"
#include "System/DiaSaveGame.h"
#include "System/DiaRewardSubsystem.h"
#include "System/DiaMapGeneratorSubsystem.h"
#include "DiaGameState.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "Engine/GameInstance.h"
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

    const ADiaGameState* GS = World->GetGameState<ADiaGameState>();
    const UDiaRewardSubsystem* Rewards = GetGameInstance()->GetSubsystem<UDiaRewardSubsystem>();
    if (PendingLoadedGame || (GS && GS->IsInBattle()) || (Rewards && Rewards->IsApplyingReward()))
    {
        UE_LOG(LogARPG, Warning, TEXT("Save rejected while loading, fighting, or applying a reward."));
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

    if (!PlayerCharacter->GetAttributeSet() || PlayerCharacter->GetAttributeSet()->GetHealth() <= 0.f) return false;
    SaveGameInstance->SaveVersion = 1;
	SaveGameInstance->PlayerName = TEXT("PlayerOne");
	SaveGameInstance->SaveLevelName = UGameplayStatics::GetCurrentLevelName(World, true);
	FillSaveDataByGame(SaveGameInstance, Controller);

    if (SaveGameInstance->bHasDungeonFloor && !SaveGameInstance->IsFloorSnapshotValid())
    {
        UE_LOG(LogARPG, Error, TEXT("Save rejected: floor snapshot validation failed."));
        return false;
    }
	return UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0);
}

bool UDiaSaveSubsystem::LoadGame(const FString& SlotName)
{
    const UDiaRewardSubsystem* Rewards = GetGameInstance()->GetSubsystem<UDiaRewardSubsystem>();
    if (PendingLoadedGame || (Rewards && Rewards->IsApplyingReward())) return false;

	if (UDiaSaveGame* LoadedGame = Cast<UDiaSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0)))
	{
        if (LoadedGame->SaveVersion > 1 || (LoadedGame->bHasDungeonFloor && !LoadedGame->IsFloorSnapshotValid()))
        {
            UE_LOG(LogARPG, Error, TEXT("Save slot has an unsupported version or invalid floor snapshot; load cancelled."));
            return false;
        }
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
			SaveGameInstance->Gold = InvenComp->GetGold();
			InvenComp->SaveInventoryToSaveGame(SaveGameInstance);
			EquipComp->SaveEquipmentToSaveGame(SaveGameInstance);
		}

		if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
		{
			if (const UDiaAttributeSet* Attr = Cast<UDiaAttributeSet>(ASC->GetAttributeSet(UDiaAttributeSet::StaticClass())))
			{
				SaveGameInstance->PlayerLevel = FMath::Max(1, FMath::RoundToInt(Attr->GetLevel()));
                SaveGameInstance->Health = Attr->GetHealth();
                SaveGameInstance->Mana = Attr->GetMana();
                SaveGameInstance->Experience = Attr->GetExp();
			}
		}

        if (const UDiaSkillManagerComponent* Skills = PlayerCharacter->FindComponentByClass<UDiaSkillManagerComponent>())
        {
            SaveGameInstance->Skills.Reset();
            for (const USkillObject* Skill : Skills->GetSkillIDMapping())
            {
                if (!IsValid(Skill)) continue;
                FDiaSavedSkill Saved;
                Saved.SkillID = Skill->GetSkillID();
                Saved.Level = Skill->GetSkillLevel();
                Saved.OwnedVariants = Skill->GetOwnedVariantIDs();
                Saved.AppliedVariants = Skill->GetVariantApplyIDs();
                SaveGameInstance->Skills.Add(Saved);
            }
            SaveGameInstance->QuickSlots = Skills->GetQuickSlotSkillIDs();
        }
        if (const UDiaMapGeneratorSubsystem* Map = Controller->GetWorld()->GetSubsystem<UDiaMapGeneratorSubsystem>())
        {
            SaveGameInstance->FloorSize = Map->GetMapSize();
            SaveGameInstance->RoomLayout = Map->GetRoomLayout();
            SaveGameInstance->StartRoom = Map->GetStartRoomData();
            SaveGameInstance->bHasDungeonFloor = SaveGameInstance->StartRoom.RoomGuid.IsValid();
        }
        if (const ADiaGameState* GS = Controller->GetWorld()->GetGameState<ADiaGameState>())
            SaveGameInstance->ClearedRooms = GS->GetClearedRoomIDs();
        if (const UDiaRewardSubsystem* Rewards = GetGameInstance()->GetSubsystem<UDiaRewardSubsystem>())
            SaveGameInstance->RoomRewards = Rewards->GetRoomRewards();
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
            // Remove initial equipment effects before rebuilding level attributes.
            TArray<EEquipmentSlot> ExistingSlots;
            EquipComp->GetEquipmentMap().GetKeys(ExistingSlots);
            for (const EEquipmentSlot Slot : ExistingSlots) Controller->OnUnequipItemProgress(Slot);
            InvenComp->LoadInventoryFromSaveGame(SaveGameInstance);
            if (SaveGameInstance->SaveVersion >= 1) InvenComp->SetGold(SaveGameInstance->Gold);
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

        if (IsValid(EquipComp))
        {
            for (const auto& Entry : SaveGameInstance->EquippedItems)
            {
                Controller->OnEquipItemProgress(Entry.Value, Entry.Key);
                EquipComp->OnItemEquipped.Broadcast(Entry.Value, Entry.Key);
            }
        }
        if (SaveGameInstance->SaveVersion >= 1)
        {
            UDiaSkillManagerComponent* Skills = PlayerCharacter->FindComponentByClass<UDiaSkillManagerComponent>();
            UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
            if (Skills && ASC)
            {
                TArray<FGameplayAbilitySpecHandle> OldHandles;
                for (const USkillObject* Skill : Skills->GetSkillIDMapping())
                    if (Skill) if (const FGameplayAbilitySpec* Spec = Skills->GetAbilitySpecBySkillID(Skill->GetSkillID())) OldHandles.Add(Spec->Handle);
                for (const FGameplayAbilitySpecHandle Handle : OldHandles) ASC->ClearAbility(Handle);
                Skills->SetSkillIDMapping(TArray<int32>());
                for (const FDiaSavedSkill& Saved : SaveGameInstance->Skills)
                {
                    if (!PlayerCharacter->ApplySkillByID(Saved.SkillID))
                    {
                        UE_LOG(LogARPG, Error, TEXT("Saved skill %d could not be restored."), Saved.SkillID);
                        continue;
                    }
                    if (Saved.Level > 1) PlayerCharacter->ApplySkillLevelUpByID(Saved.SkillID, Saved.Level - 1);
                    for (const int32 Variant : Saved.OwnedVariants) Skills->TryUnlockSkillVariantByID(Saved.SkillID, Variant);
                    for (const int32 Variant : Saved.AppliedVariants) Skills->TryApplySkillVariantByID(Saved.SkillID, Variant);
                }
                for (int32 Index = 0; Index < UDiaSkillManagerComponent::MaxQuickSlotCount; ++Index)
                {
                    const int32 SkillID = SaveGameInstance->QuickSlots.IsValidIndex(Index) ? SaveGameInstance->QuickSlots[Index] : -1;
                    Skills->SetSkillIDIndex(SkillID, Index);
                    Controller->RegisteSkillOnQuickSlotWidget(SkillID, Index);
                }
            }
            if (UDiaAttributeSet* Attr = PlayerCharacter->GetAttributeSet())
            {
                Attr->SetExp(FMath::Max(0.f, SaveGameInstance->Experience));
                Attr->SetHealth(FMath::Clamp(SaveGameInstance->Health, 0.f, Attr->GetMaxHealth()));
                Attr->SetMana(FMath::Clamp(SaveGameInstance->Mana, 0.f, Attr->GetMaxMana()));
            }
        }
        // Legacy saves contain no reproducible layout. Use the new floor's entrance.
        if (SaveGameInstance->SaveVersion >= 1 && (!SaveGameInstance->bHasDungeonFloor || SaveGameInstance->IsFloorSnapshotValid()))
            PlayerCharacter->SetActorLocation(SaveGameInstance->PlayerLocation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}
