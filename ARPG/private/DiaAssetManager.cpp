// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaAssetManager.h"
#include "GAS/DiaGameplayTags.h"

UDiaAssetManager& UDiaAssetManager::Get()
{
	check(GEngine);
	UDiaAssetManager* DiaAssetManager = Cast<UDiaAssetManager>(GEngine->AssetManager);
	return *DiaAssetManager;
}

void UDiaAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FDiaGameplayTags::InitializeNativeTags();

	UE_LOG(LogTemp, Log, TEXT("DiaAssetManager: Native Gameplay Tags have been initialized."));
}