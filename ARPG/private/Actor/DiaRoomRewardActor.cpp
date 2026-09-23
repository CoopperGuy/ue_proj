#include "Actor/DiaRoomRewardActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Camera/PlayerCameraManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Controller/DiaController.h"
#include "DiaGameState.h"
#include "GameMode/DungeonGameMode.h"
#include "System/DiaRewardSubsystem.h"
#include "UI/HUDWidget.h"

ADiaRoomRewardActor::ADiaRoomRewardActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RewardHitBox"));
    SetRootComponent(HitBox);
    HitBox->SetBoxExtent(FVector(65.f, 65.f, 75.f));
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RewardPedestal"));
    Mesh->SetupAttachment(HitBox);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Shape(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (Shape.Succeeded()) Mesh->SetStaticMesh(Shape.Object);
    Mesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.6f));
    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("RewardLabel"));
    Label->SetupAttachment(HitBox);
    Label->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetWorldSize(24.f);
    Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("RewardLight"));
    Light->SetupAttachment(HitBox);
    Light->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
    Light->SetIntensity(3500.f);
    Light->SetAttenuationRadius(300.f);
    Light->SetCastShadows(false);
}

void ADiaRoomRewardActor::InitializeReward(const FGuid& InRoom, EDiaRoomRewardKind InKind)
{
    RoomGuid = InRoom;
    Kind = InKind;
    FString Text = Kind == EDiaRoomRewardKind::Boss ? TEXT("BOSS REWARD - CHOOSE ONE")
        : Kind == EDiaRoomRewardKind::Treasure ? TEXT("TREASURE - CLICK TO INSPECT") : TEXT("PICKUP");
    FColor Color = Kind == EDiaRoomRewardKind::Boss ? FColor(180, 100, 255) : FColor(255, 210, 65);
    const UDiaRewardSubsystem* Rewards = GetGameInstance()->GetSubsystem<UDiaRewardSubsystem>();
    const FDiaRoomRewardState* State = Rewards ? Rewards->FindRoomReward(RoomGuid) : nullptr;
    if (State && State->bClaimed && Kind == EDiaRoomRewardKind::Boss) Text = TEXT("FLOOR CLEARED - CONTINUE");
    else if (Kind == EDiaRoomRewardKind::Normal && State && !State->Options.IsEmpty())
    {
        const FRewardData& Reward = State->Options[0];
        if (Reward.RewardType == ERewardType::Gold) Text = FString::Printf(TEXT("GOLD +%d"), Reward.GoldAmount);
        else if (Reward.RewardType == ERewardType::Heal) { Text = TEXT("HEAL 25%"); Color = FColor(100, 255, 140); }
        else Text = TEXT("EQUIPMENT");
        Mesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.25f));
    }
    Label->SetText(FText::FromString(Text));
    Label->SetTextRenderColor(Color);
    Light->SetLightColor(FLinearColor(Color));
}

void ADiaRoomRewardActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->PlayerCameraManager)
        Label->SetWorldRotation((PC->PlayerCameraManager->GetCameraLocation() - Label->GetComponentLocation()).Rotation());
}

void ADiaRoomRewardActor::OnInteract(APlayerController* Controller)
{
    ADiaController* PC = Cast<ADiaController>(Controller);
    const ADiaGameState* GS = GetWorld()->GetGameState<ADiaGameState>();
    if (!PC || !PC->GetPawn() || !GS || GS->IsInBattle()) return;
    if (FVector::DistSquared2D(PC->GetPawn()->GetActorLocation(), GetActorLocation()) > FMath::Square(350.f))
    {
        Label->SetText(FText::FromString(TEXT("MOVE CLOSER TO PICK UP")));
        return;
    }
    UDiaRewardSubsystem* Rewards = GetGameInstance()->GetSubsystem<UDiaRewardSubsystem>();
    const FDiaRoomRewardState* State = Rewards->ResolveRoomReward(RoomGuid, Kind, PC);
    if (!State) { Label->SetText(FText::FromString(TEXT("REWARD DATA UNAVAILABLE"))); return; }
    ADungeonGameMode* Mode = GetWorld()->GetAuthGameMode<ADungeonGameMode>();
    if (State->bClaimed)
    {
        if (Mode && Kind == EDiaRoomRewardKind::Boss) Mode->OnRoomRewardClaimed(RoomGuid);
        else Destroy();
        return;
    }
    if (State->Options.IsEmpty()) { Destroy(); return; }
    if (Kind == EDiaRoomRewardKind::Normal)
    {
        if (Rewards->TryClaimRoomReward(RoomGuid, 0, PC))
        {
            if (Mode) Mode->OnRoomRewardClaimed(RoomGuid);
            else Destroy();
        }
        else Label->SetText(FText::FromString(TEXT("CHECK HEALTH / INVENTORY SPACE")));
    }
    else if (UHUDWidget* HUD = PC->GetHUDWidget())
    {
        HUD->OpenRewardChoicePanel(
            Kind == EDiaRoomRewardKind::Boss ? NSLOCTEXT("RoomReward", "BossTitle", "보스 보상") : NSLOCTEXT("RoomReward", "TreasureTitle", "보물방"),
            Kind == EDiaRoomRewardKind::Boss ? NSLOCTEXT("RoomReward", "BossSubtitle", "두 보상 중 하나를 선택하세요.") : NSLOCTEXT("RoomReward", "TreasureSubtitle", "보상을 확인하고 획득하세요."),
            State->Options);
    }
}
