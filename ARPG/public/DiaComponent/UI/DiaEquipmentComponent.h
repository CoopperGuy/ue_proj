// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ItemBase.h"
#include "DiaEquipmentComponent.generated.h"



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

private:
	//장착용 아이템 맵 -> FInventoryItem에서 다른 구조체 생성 해야한다.
	TMap<EEquipmentSlot, FInventorySlot> EquipmentMap;
		
	
};
