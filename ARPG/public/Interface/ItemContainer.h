// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/ItemBase.h"
#include "ItemContainer.generated.h"

class UItemWidget;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UItemContainer : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPG_API IItemContainer
{
	GENERATED_BODY()

public:
	virtual bool AddItem(const FInventorySlot& ItemInstance, UItemWidget* ItemWidget, int32 PosY = -1, int32 PosX = -1) = 0;
};
