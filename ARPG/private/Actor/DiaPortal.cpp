// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/DiaPortal.h"

#include "Controller/DiaController.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "GameMode/DungeonGameMode.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Logging/ARPGLogChannels.h"

ADiaPortal::ADiaPortal()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.TickInterval = 0.1f;

	PortalCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalCollision"));
	SetRootComponent(PortalCollision);
	PortalCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PortalCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	PortalCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	PortalCollision->SetGenerateOverlapEvents(true);

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(RootComponent);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DestinationLevel = NAME_None;
}

void ADiaPortal::OnInteract(APlayerController* InteractingController)
{
	if (ADungeonGameMode* GameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->HandlePortalInteraction(this, InteractingController);
	}
	else
	{
		UE_LOG(LogARPG_Inventory, Warning, TEXT("OnInteract failed: DungeonGameMode is null."));
	}
}

void ADiaPortal::BeginPlay()
{
	Super::BeginPlay();
	// AActor::OnClicked는 PC가 bEnableMouseOverEvents일 때 CurrentClickablePrimitive만 사용해
}

void ADiaPortal::OnPortalCursorOver()
{
	if (ADiaController* DiaPC = Cast<ADiaController>(GetWorld()->GetFirstPlayerController()))
	{
		DiaPC->CurrentMouseCursor = EMouseCursor::Hand;
	}
}

void ADiaPortal::OnPortalCursorOut()
{
	if (ADiaController* DiaPC = Cast<ADiaController>(GetWorld()->GetFirstPlayerController()))
	{
		DiaPC->CurrentMouseCursor = EMouseCursor::Default;
	}
}

