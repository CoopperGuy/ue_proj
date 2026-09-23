#pragma once

#include "CoreMinimal.h"
#include "Types/DiaRewardData.h"
#include "DiaRoomRewardState.generated.h"

UENUM(BlueprintType)
enum class EDiaRoomRewardKind : uint8 { Normal, Treasure, Boss };

USTRUCT()
struct ARPG_API FDiaRoomRewardState
{
    GENERATED_BODY()
    UPROPERTY() EDiaRoomRewardKind Kind = EDiaRoomRewardKind::Normal;
    // Resolved with zero options is a deliberate no-drop result.
    UPROPERTY() bool bResolved = false;
    UPROPERTY() bool bClaimed = false;
    UPROPERTY() TArray<FRewardData> Options;
};
