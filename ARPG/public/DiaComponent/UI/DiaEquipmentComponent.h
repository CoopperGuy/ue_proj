// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ItemBase.h"
#include "DiaEquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUnEquipped, EEquipmentSlot, SlotType);

class UEquipWidget;

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
	void EquipItem(const FEquippedItem& Item, EEquipmentSlot Slot);
	UFUNCTION()
	void UnEquipItem(EEquipmentSlot Slot);

	const FEquippedItem* GetEquippedItem(EEquipmentSlot Slot) const;
public:
	FOnItemUnEquipped OnItemUnEquipped;
private:
	//장착한 아이템을 매핑하는 컴포넌트
	UPROPERTY()
	TMap<EEquipmentSlot, FEquippedItem> EquipmentMap;
};
