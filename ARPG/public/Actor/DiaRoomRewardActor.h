#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/DiaInteractable.h"
#include "Types/DiaRoomRewardState.h"
#include "DiaRoomRewardActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UPointLightComponent;
class UStaticMesh;

// Native fallback presentation: no additional Blueprint asset is required.
UCLASS()
class ARPG_API ADiaRoomRewardActor : public AActor, public IDiaInteractable
{
    GENERATED_BODY()
public:
    ADiaRoomRewardActor();
    void InitializeReward(const FGuid& InRoom, EDiaRoomRewardKind InKind);
    virtual bool IsInteractable() const override { return true; }
    virtual void OnInteract(APlayerController* Controller) override;
    virtual void Tick(float DeltaSeconds) override;
private:
    UPROPERTY() FGuid RoomGuid;
    UPROPERTY() EDiaRoomRewardKind Kind = EDiaRoomRewardKind::Normal;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> HitBox;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UTextRenderComponent> Label;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UPointLightComponent> Light;
    UPROPERTY(EditDefaultsOnly, Category = "Reward|Presentation") TObjectPtr<UStaticMesh> AltarMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Reward|Presentation") TObjectPtr<UStaticMesh> TreasureMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Reward|Presentation") TObjectPtr<UStaticMesh> GoldMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Reward|Presentation") TObjectPtr<UStaticMesh> HealMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Reward|Presentation") TObjectPtr<UStaticMesh> EquipmentMesh;
    UPROPERTY(EditDefaultsOnly, Category = "Reward|Presentation") TObjectPtr<UStaticMesh> ExitMesh;
};
