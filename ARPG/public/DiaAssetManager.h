// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "DiaAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UDiaAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
public:
	static UDiaAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
	
	
};
