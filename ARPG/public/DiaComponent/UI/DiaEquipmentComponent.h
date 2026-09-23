// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ItemBase.h"
#include "DiaEquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUnEquipped, EEquipmentSlot, SlotType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemEquipped, const FEquippedItem&, Item, EEquipmentSlot, SlotType);

class UEquipWidget;
class UDiaSaveGame;
class UDiaInventoryComponent;
class UDiaOptionManagerComponent;

USTRUCT(BlueprintType)
struct ARPG_API FEquipmentDebugEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Sequence = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bPassed = false;

	UPROPERTY(BlueprintReadOnly)
	FString Message;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UDiaEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaEquipmentComponent();

protected:
	virtual void BeginPlay() override;
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//아이템이 드래그 되면 호출하여 아이템을 장착한다.
	UFUNCTION()
	void EquipItem(const FEquippedItem& Item, EEquipmentSlot SlotType);
	UFUNCTION()
	void UnEquipItem(EEquipmentSlot SlotType);
	void UnEquipItemFinish(EEquipmentSlot SlotType);

	const FEquippedItem* GetEquippedItem(EEquipmentSlot SlotType) const;
	void AddDebugEvent(const FString& Message, bool bPassed);

	void SaveEquipmentToSaveGame(UDiaSaveGame* SaveGameInstance) const;
	void LoadEquipmentFromSaveGame(const UDiaSaveGame* SaveGameInstance);

	bool TryEquipFromInventory(const FInventorySlot& InventorySlot, EEquipmentSlot SlotType);
	bool TryUnEquipToInventory(EEquipmentSlot SlotType, FInventorySlot& OutInventorySlot, int32 PosX = -1, int32 PosY = -1, bool bNotify = true);
public:
	FOnItemEquipped OnItemEquipped;
	FOnItemUnEquipped OnItemUnEquipped;
private:
	//장착한 아이템을 매핑하는 컴포넌트
	UPROPERTY()
	TMap<EEquipmentSlot, FEquippedItem> EquipmentMap;

	UPROPERTY()
	TArray<FEquipmentDebugEvent> DebugEvents;

	UPROPERTY()
	TObjectPtr<UDiaInventoryComponent> InventoryComponent;

	UPROPERTY()
	TObjectPtr<UDiaOptionManagerComponent> OptionManagerComponent;

	UPROPERTY()
	int32 DebugEventSequence = 0;

public:
	FORCEINLINE const TMap<EEquipmentSlot, FEquippedItem>& GetEquipmentMap() const { return EquipmentMap; }
	FORCEINLINE const TArray<FEquipmentDebugEvent>& GetDebugEvents() const { return DebugEvents; }
	FORCEINLINE void SetInventoryComponent(UDiaInventoryComponent* InventoryComp) { InventoryComponent = InventoryComp; }
	FORCEINLINE void SetOptionManagerComponent(UDiaOptionManagerComponent* OptionManagerComp) { OptionManagerComponent = OptionManagerComp; }
};
