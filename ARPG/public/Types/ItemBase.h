#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemBase.generated.h"  

class UTexture2D;
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Common,
	EIR_Uncommon,
	EIR_Rare,
	EIR_Epic,
	EIR_Legendary,
	EIR_Mythic,
	EIR_MAX
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Weapon,
	EIT_Armor,
	EIT_Consumable,
	EIT_Misc,
	EIT_MAX
};


USTRUCT(BlueprintType)
struct ARPG_API FItemBase : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;
    
    // 그리드 크기
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Width = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Height = 1;
    
    // 아이템 등급
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemRarity Rarity = EItemRarity::EIR_Common;
    
    // 아이템 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemType ItemType = EItemType::EIT_Misc;
    
    // 스택 관련
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bStackable = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* ItemIcon = nullptr;
};


USTRUCT(BlueprintType)
struct ARPG_API FGrid
{
    GENERATED_BODY()

    TArray<bool> Cells;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Width;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Height;

    FGrid(int32 InWidth, int32 InHeight)
        : Width(InWidth), Height(InHeight)
    {
        Cells.SetNum(Width * Height);
    }

    bool& operator()(int32 X, int32 Y)
    {
        return Cells[Y * Width + X];
    }

    const bool& operator()(int32 X, int32 Y) const
    {
        return Cells[Y * Width + X];
    }
};