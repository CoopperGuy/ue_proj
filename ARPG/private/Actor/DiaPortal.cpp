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
	PrimaryActorTick.bCanEverTick = true;
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
}

void ADiaPortal::BeginPlay()
{
	Super::BeginPlay();
	// AActor::OnClicked는 PC가 bEnableMouseOverEvents일 때 CurrentClickablePrimitive만 사용해
	// 캡처/호버 갱신이 어긋나면 클릭이 안 들어옴. 아래 Tick에서 커서 트레이스 + JustPressed로 처리.
}

void ADiaPortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!IsValid(PC) || !PC->bShowMouseCursor)
	{
		return;
	}

	FHitResult Hit;
	const bool bTraceHit = PC->GetHitResultUnderCursor(ECC_Visibility, true, Hit);
	const bool bOverPortal = bTraceHit && Hit.Component.Get() == PortalCollision;

	if (bOverPortal && bCursorHoveringPortal && PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		OnPortalClicked(this, EKeys::LeftMouseButton);
	}

	if (bOverPortal && !bCursorHoveringPortal)
	{
		bCursorHoveringPortal = true;
		OnPortalCursorOver();
	}
	else if (!bOverPortal && bCursorHoveringPortal)
	{
		bCursorHoveringPortal = false;
		OnPortalCursorOut();
	}
}

void ADiaPortal::OnPortalCursorOver()
{
	if (ADiaController* DiaPC = Cast<ADiaController>(GetWorld()->GetFirstPlayerController()))
	{
		DiaPC->CurrentMouseCursor = EMouseCursor::Hand;
		DiaPC->SetBlockSkillInput(true);
	}
}

void ADiaPortal::OnPortalCursorOut()
{
	if (ADiaController* DiaPC = Cast<ADiaController>(GetWorld()->GetFirstPlayerController()))
	{
		DiaPC->CurrentMouseCursor = EMouseCursor::Default;
		DiaPC->SetBlockSkillInput(false);
	}
}

void ADiaPortal::OnPortalClicked(AActor* TouchedActor, FKey ButtonPressed)
{
	ADungeonGameMode* GameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
	UE_LOG(LogARPG, Log, TEXT("Portal clicked, attempting to warp to TestRoom"));
	if (!IsValid(GameMode))
	{
		return;
	}
	UE_LOG(LogARPG, Log, TEXT("Portal clicked, attempting to warp to TestRoom"));
	GameMode->WarpOtherLevel("TestRoom");
}
