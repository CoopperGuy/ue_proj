#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapGenerate.generated.h"

UENUM(BlueprintType)
enum class EDiaDirection : uint8
{
	North,
	East,
	South,
	West,
	None
};

// 대응 방향 반환: North↔South, East↔West
inline EDiaDirection GetOppositeDirection(EDiaDirection Direction)
{
	switch (Direction)
	{
	case EDiaDirection::North: return EDiaDirection::South;
	case EDiaDirection::East: return EDiaDirection::West;
	case EDiaDirection::South: return EDiaDirection::North;
	case EDiaDirection::West: return EDiaDirection::East;
	default: return EDiaDirection::None;
	}
}

USTRUCT(BlueprintType)
struct FDiaAdjacencyRule : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SourceRoomID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EDiaDirection> Directions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> CandidateWeights;
};

