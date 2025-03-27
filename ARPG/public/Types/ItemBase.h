#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemBase.generated.h"  

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

UENUM(BlueprintType)
enum class EItemStat : uint8
{
	EIS_Health,
	EIS_Mana,
	EIS_Attack,
	EIS_Defense,
	EIS_Speed,
	EIS_MAX
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
    
    // �׸��� ũ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Width = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Height = 1;
    
    // ������ ���
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemRarity Rarity = EItemRarity::EIR_Common;
    
    // ������ Ÿ��
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemType ItemType = EItemType::EIT_Misc;
        
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxStackSize = 1;

    // ���� ����
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bStackable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FSoftObjectPath IconPath;
};


USTRUCT(BlueprintType)
struct ARPG_API FGrid
{
    GENERATED_BODY()

	FGrid() :Width(5), Height(5) 
    {
		Cells.SetNum(Width * Height);
    }

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

//// ������ ���λ�/���̻� (��ƺ��� ��Ÿ��)
//USTRUCT(BlueprintType)
//struct YOURGAME_API FItemAffix
//{
//    GENERATED_BODY()
//    
//    // ���λ�/���̻� ID
//    UPROPERTY(EditAnywhere, BlueprintReadWrite)
//    FName AffixID;
//    
//    // ���λ�(true) �Ǵ� ���̻�(false)
//    UPROPERTY(EditAnywhere, BlueprintReadWrite)
//    bool bIsPrefix = true;
//    
//    // �ο��ϴ� ȿ�� (����, ��ġ)
//    UPROPERTY(EditAnywhere, BlueprintReadWrite)
//    TMap<EItemStat, float> StatModifiers;
//    
//    // �ؽ�Ʈ ǥ�ÿ�
//    UPROPERTY(EditAnywhere, BlueprintReadWrite)
//    FText DisplayText;
//};
//
//// ������ ���� (��ƺ��� ��Ÿ��)
//USTRUCT(BlueprintType)
//struct YOURGAME_API FItemSocket
//{
//    GENERATED_BODY()
//    
//    // ���� Ÿ�� (���, ���޶��� ��)
//    UPROPERTY(EditAnywhere, BlueprintReadWrite)
//    ESocketType SocketType = ESocketType::Empty;
//    
//    // ���Ե� ���� ID (������ �������)
//    UPROPERTY(EditAnywhere, BlueprintReadWrite)
//    FName GemItemID;
//    
//    // �ο��� ȿ��
//    UPROPERTY(EditAnywhere, BlueprintReadWrite)
//    TArray<FItemEffect> SocketEffects;
//};

USTRUCT(BlueprintType)
struct ARPG_API FInventoryItem
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FGuid InstanceID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Quantity = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Grid")
    int32 GridX = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Grid")
    int32 GridY = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Grid")
    int32 Level = 0;
    
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Affixes")
    //TArray<FItemAffix> Affixes;
    
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Sockets")
    //TArray<FItemSocket> Sockets;
            
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bIsLocked = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bRandomStats = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText CustomDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TMap<EItemStat, float> Stats;

    FInventoryItem()
        : InstanceID(FGuid::NewGuid())
    {
    }
    
    //FItemBase* GetBaseItemData() const;
    
    bool IsValid() const { return !ItemID.IsNone() && Quantity > 0; }
    
    //void GetItemSize(int32& OutWidth, int32& OutHeight) const;
    
    FText GenerateTooltip() const { return FText(); };
};