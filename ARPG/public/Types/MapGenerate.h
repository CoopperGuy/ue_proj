#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MapGenerate.generated.h"

/** 맵/방 타일 크기 - 한 곳에서만 변경 */
namespace DiaMapConstants
{
	constexpr float TileSize = 1000.f;
	constexpr float HalfTileSize = TileSize * 0.5f;
}

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
	Boss,
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
struct FDiaRoomPort
{
	GENERATED_BODY()

	FDiaRoomPort() = default;

	FDiaRoomPort(const FName& InPortID, const FIntPoint& InLocalCell, EDiaDirection InDirection)
		: PortID(InPortID)
		, LocalCell(InLocalCell)
		, Direction(InDirection)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PortID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntPoint LocalCell = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDiaDirection Direction = EDiaDirection::North;

	bool operator==(const FDiaRoomPort& Other) const
	{
		return LocalCell == Other.LocalCell
			&& Direction == Other.Direction;
	}
};

USTRUCT(BlueprintType)
struct FDiaAdjacencyRule : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SourceRoomID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoomWeight> CandidateWeights;
};

namespace DiaMapGenerator
{
	constexpr uint8 NorthBit = 1 << 0; // 0001
	constexpr uint8 EastBit = 1 << 1;  // 0010
	constexpr uint8 SouthBit = 1 << 2; // 0100
	constexpr uint8 WestBit = 1 << 3;  // 1000
	constexpr uint8 AllDirectionsBit = NorthBit | EastBit | SouthBit | WestBit; // 1111

	inline int32 NormalizeRotateDegree(int32 Degree)
	{
		return ((FMath::RoundToInt(Degree / 90.f) % 4) + 4) % 4;
	}

	inline FIntPoint GetDirectionOffset(EDiaDirection Direction)
	{
		switch (Direction)
		{
		case EDiaDirection::North:
			return FIntPoint(1, 0);
		case EDiaDirection::East:
			return FIntPoint(0, 1);
		case EDiaDirection::South:
			return FIntPoint(-1, 0);
		case EDiaDirection::West:
			return FIntPoint(0, -1);
		default:
			return FIntPoint::ZeroValue;
		}
	}

	inline FIntPoint GetSafeRoomSize(const FIntPoint& RoomSize)
	{
		return FIntPoint(FMath::Max(1, RoomSize.X), FMath::Max(1, RoomSize.Y));
	}

	inline bool IsLocalCellInRoom(const FIntPoint& LocalCell, const FIntPoint& RoomSize)
	{
		const FIntPoint SafeSize = GetSafeRoomSize(RoomSize);
		return LocalCell.X >= 0
			&& LocalCell.X < SafeSize.X
			&& LocalCell.Y >= 0
			&& LocalCell.Y < SafeSize.Y;
	}

	inline bool IsPortOnBoundary(const FDiaRoomPort& Port, const FIntPoint& RoomSize)
	{
		if (Port.Direction == EDiaDirection::None || !IsLocalCellInRoom(Port.LocalCell, RoomSize))
		{
			return false;
		}

		const FIntPoint SafeSize = GetSafeRoomSize(RoomSize);
		switch (Port.Direction)
		{
		case EDiaDirection::North:
			return Port.LocalCell.X == SafeSize.X - 1;
		case EDiaDirection::East:
			return Port.LocalCell.Y == SafeSize.Y - 1;
		case EDiaDirection::South:
			return Port.LocalCell.X == 0;
		case EDiaDirection::West:
			return Port.LocalCell.Y == 0;
		default:
			return false;
		}
	}

	inline FIntPoint GetRotatedRoomSize(const FIntPoint& RoomSize, int32 RotateDegree)
	{
		const FIntPoint SafeSize = GetSafeRoomSize(RoomSize);
		const int32 RotateSteps = NormalizeRotateDegree(RotateDegree);
		return RotateSteps % 2 == 0 ? SafeSize : FIntPoint(SafeSize.Y, SafeSize.X);
	}

	inline EDiaDirection RotateDirectionDegree(int32 Degree, EDiaDirection Direction)
	{
		if (Direction == EDiaDirection::None)
		{
			return EDiaDirection::None;
		}

		const int32 RotateSteps = NormalizeRotateDegree(Degree);
		const int32 RotatedIndex = (static_cast<int32>(Direction) + RotateSteps) % 4;
		return static_cast<EDiaDirection>(RotatedIndex);
	}

	inline uint8 SetDirection(EDiaDirection Direction)
	{
		if (Direction == EDiaDirection::None)
		{
			return 0;
		}

		uint8 Directions = 0;
		Directions |= (1 << static_cast<uint8>(Direction));
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
		int32 RotateSteps = NormalizeRotateDegree(Degree);
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

	inline FIntPoint RotateLocalCellDegree(const FIntPoint& LocalCell, const FIntPoint& RoomSize, int32 Degree)
	{
		const FIntPoint SafeSize = GetSafeRoomSize(RoomSize);
		const FIntPoint ClampedCell(
			FMath::Clamp(LocalCell.X, 0, SafeSize.X - 1),
			FMath::Clamp(LocalCell.Y, 0, SafeSize.Y - 1));

		switch (NormalizeRotateDegree(Degree))
		{
		case 1:
			return FIntPoint(SafeSize.Y - 1 - ClampedCell.Y, ClampedCell.X);
		case 2:
			return FIntPoint(SafeSize.X - 1 - ClampedCell.X, SafeSize.Y - 1 - ClampedCell.Y);
		case 3:
			return FIntPoint(ClampedCell.Y, SafeSize.X - 1 - ClampedCell.X);
		default:
			return ClampedCell;
		}
	}

	inline FDiaRoomPort RotatePortDegree(const FDiaRoomPort& Port, const FIntPoint& RoomSize, int32 Degree)
	{
		FDiaRoomPort RotatedPort = Port;
		RotatedPort.LocalCell = RotateLocalCellDegree(Port.LocalCell, RoomSize, Degree);
		RotatedPort.Direction = RotateDirectionDegree(Degree, Port.Direction);
		return RotatedPort;
	}

	inline TArray<FDiaRoomPort> MakeDefaultPorts(const FIntPoint& RoomSize)
	{
		const FIntPoint SafeSize = GetSafeRoomSize(RoomSize);
		const int32 CenterX = FMath::Clamp((SafeSize.X - 1) / 2, 0, SafeSize.X - 1);
		const int32 CenterY = FMath::Clamp((SafeSize.Y - 1) / 2, 0, SafeSize.Y - 1);

		TArray<FDiaRoomPort> Ports;
		Ports.Reserve(4);
		Ports.Add({ TEXT("North"), FIntPoint(SafeSize.X - 1, CenterY), EDiaDirection::North });
		Ports.Add({ TEXT("East"), FIntPoint(CenterX, SafeSize.Y - 1), EDiaDirection::East });
		Ports.Add({ TEXT("South"), FIntPoint(0, CenterY), EDiaDirection::South });
		Ports.Add({ TEXT("West"), FIntPoint(CenterX, 0), EDiaDirection::West });
		return Ports;
	}

	inline uint8 MakeDirectionByPorts(const TArray<FDiaRoomPort>& Ports, const FIntPoint& RoomSize, int32 RotateDegree)
	{
		uint8 Directions = 0;
		for (const FDiaRoomPort& Port : Ports)
		{
			if (!IsPortOnBoundary(Port, RoomSize))
			{
				continue;
			}

			const FDiaRoomPort RotatedPort = RotatePortDegree(Port, RoomSize, RotateDegree);
			Directions |= SetDirection(RotatedPort.Direction);
		}
		return Directions;
	}

	inline uint32 CountSetBits(uint8 Value)
	{
		uint32 Count = 0;
		for (uint8 i = 0; i < 4; ++i)
		{
			if (Value & (1 << i))
			{
				++Count;
			}
		}
		return Count;
	}

}

