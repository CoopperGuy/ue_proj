// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ItemDragDropSubsystem.generated.h"

class UItemDragDropOperation;
/**
 * 
 */
UCLASS()
class ARPG_API UItemDragDropSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
public:
	void OnDragStarted(UItemDragDropOperation* DragOp);
	void OnDragEnded();
	
	
	
};
