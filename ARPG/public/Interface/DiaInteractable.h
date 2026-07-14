// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "DiaInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDiaInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ARPG_API IDiaInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool IsInteractable() const = 0;
	virtual void OnInteract(APlayerController* InteractingController) = 0;
	
};
