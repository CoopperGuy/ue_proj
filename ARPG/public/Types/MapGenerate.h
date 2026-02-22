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

UENUM(BlueprintType)
enum class ETileType : uint8
{
	Floor,
	Corridor,
	Empty
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

// 모든 방향을 순회하기 위한 헬퍼 (None 제외)
inline TArray<EDiaDirection> GetAllDirections()
{
	return { EDiaDirection::North, EDiaDirection::East, EDiaDirection::South, EDiaDirection::West };
}

USTRUCT(BlueprintType)
struct FRoomWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RoomID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 0.f;
};

USTRUCT(BlueprintType)
struct FDiaAdjacencyRule : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SourceRoomID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EDiaDirection> Directions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoomWeight> CandidateWeights;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETileType TileType = ETileType::Empty;
};

namespace DiaMapGenerator
{
	constexpr uint8 NorthBit = 1 << 3; // 1000
	constexpr uint8 EastBit = 1 << 2;  // 0100
	constexpr uint8 SouthBit = 1 << 1; // 0010
	constexpr uint8 WestBit = 1 << 0;  // 0001
	constexpr uint8 AllDirectionsBit = NorthBit | EastBit | SouthBit | WestBit; // 1111

	inline uint8 SetDirection(EDiaDirection Direction)
	{
		uint8 Directions = 0;
		Directions |= (1 << static_cast<uint8>(Direction));
		return Directions;
	}

	inline uint8 MakeDirectionByArray(const TArray<EDiaDirection>& DirectionArray)
	{
		uint8 Directions = 0;
		for (EDiaDirection Dir : DirectionArray)
		{
			Directions |= (1 << static_cast<uint8>(Dir));
		}
		return Directions;
	}

	inline TArray<EDiaDirection> GetDirections(uint8 Directions)
	{
		TArray<EDiaDirection> Result;
		for (uint8 i = 0; i < 4; ++i)
		{
			if (Directions & (1 << i))
			{
				Result.Add(static_cast<EDiaDirection>(i));
			}
		}
		return Result;
	}

	inline uint8 RotateDirectionsDegree(int32 Degree, uint8 Directions)
	{
		uint32 RotatedDirections = 0;
		int32 RotateSteps = FMath::RoundToInt(Degree / 90.f);
		for (uint8 i = 0; i < 4; ++i)
		{
			if (Directions & (1 << i))
			{
				uint8 RotatedIndex = (i + RotateSteps) % 4;
				RotatedDirections |= (1 << RotatedIndex);
			}
		}

		return RotatedDirections;
	}

}

