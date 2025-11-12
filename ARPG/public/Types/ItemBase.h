#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Types/DiaitemOptionRow.h"
#include "ItemBase.generated.h"  

static constexpr int32 MAX_PREFIX_OPTIONS = 3;
static constexpr int32 MAX_SUFFIX_OPTIONS = 3;

struct FEquippedItem;

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
	EIS_Str,
	EIS_Int,
	EIS_Dex,
	EIS_Con,
	EIS_Max,
	EIS_MAX
};


// enum class를 정수로 변환하는 유틸리티 함수
FORCEINLINE constexpr int32 ToInt(EItemStat StatType)
{
	return static_cast<int32>(StatType);
}

// 스탯 배열 크기를 가져오는 상수
static constexpr int32 STAT_ARRAY_SIZE = ToInt(EItemStat::EIS_MAX);


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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Width = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Height = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemRarity Rarity = EItemRarity::EIR_Common;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemType ItemType = EItemType::EIT_Misc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEquipmentSlot EquipmentSlot = EEquipmentSlot::EES_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEquippable = false;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStackable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSoftObjectPath IconPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> ItemMesh;

	// 기본 스탯 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EItemStat, float> BaseStats;

	//아이템 옵션이 붙을 수 있는 Tag List 불가능한 옵션도 함께 체크된다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer PossibleItemOptionTags;
};

///실존하는 아이템 값
USTRUCT(BlueprintType)
struct ARPG_API FItemInstance
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemBase BaseItem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGuid InstanceID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Level = 1;
			
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bIsLocked = false;

	//아이템 옵션들 정리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FDiaItemOptionRow> PrefixOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FDiaItemOptionRow> SubfixOptions;

	FItemInstance()
		: InstanceID(FGuid::NewGuid())
	{
		PrefixOptions.Reserve(MAX_PREFIX_OPTIONS);
		SubfixOptions.Reserve(MAX_SUFFIX_OPTIONS);
	}

	bool IsValid() const { return !BaseItem.ItemID.IsNone() && Quantity > 0; }

	FText GetDisplayName() const { return BaseItem.Name; }
	FText GetDescription() const { return BaseItem.Description; }
	int32 GetWidth() const { return BaseItem.Width; }
	int32 GetHeight() const { return BaseItem.Height; }
	EItemRarity GetRarity() const { return BaseItem.Rarity; }
	EItemType GetItemType() const { return BaseItem.ItemType; }
	bool IsEquippable() const { return BaseItem.bIsEquippable; }
	EEquipmentSlot GetEquipmentSlot() const { return BaseItem.EquipmentSlot; }
	FSoftObjectPath GetIconPath() const { return BaseItem.IconPath; }

	bool CheckPrefixOptionsSize() const { return PrefixOptions.Num() <= MAX_PREFIX_OPTIONS; }
	bool CheckSuffixOptionsSize() const { return SubfixOptions.Num() <= MAX_SUFFIX_OPTIONS; }

	static FItemInstance FromDataTable(UDataTable* ItemTable, FName ItemID, int32 InQuantity = 1)
	{
		FItemInstance Result;
		
		if (ItemTable)
		{
			if (FItemBase* FoundItem = ItemTable->FindRow<FItemBase>(ItemID, TEXT("FItemInstance::FromDataTable")))
			{
				Result.BaseItem = *FoundItem;
				Result.Quantity = InQuantity;
			}
		}
		
		return Result;
	}

	static FItemInstance FromBase(const FItemBase& _BaseItem, int32 InQuantity = 1)
	{
		FItemInstance Result;
		Result.BaseItem = _BaseItem;
		Result.Quantity = InQuantity;
		return Result;
	}
};

// 인벤토리 슬롯
USTRUCT(BlueprintType)
struct ARPG_API FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FItemInstance ItemInstance;
	
	// 그리드 위치 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridX = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridY = 0;

	bool IsEmpty() const { return !ItemInstance.IsValid(); }
	
	void Clear() 
	{ 
		ItemInstance = FItemInstance();
		GridX = GridY = 0;
	}

	static FInventorySlot FromBase(const FItemBase& _BaseItem, int32 InQuantity = 1)
	{
		FInventorySlot Result; 
		Result.ItemInstance = FItemInstance::FromBase(_BaseItem);
		return Result;
	}

	FEquippedItem ToEquippItem() const;
};

//장비용
USTRUCT(BlueprintType)
struct ARPG_API FEquippedItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FItemInstance ItemInstance;
	
	bool IsEmpty() const { return !ItemInstance.IsValid(); }
	
	void Clear() 
	{ 
		ItemInstance = FItemInstance();
	}

	// FInventorySlot에서 FEquippedItem로 변환 (그리드 정보는 손실됨)
	static FEquippedItem FromInventorySlot(const FInventorySlot& InventorySlot)
	{
		FEquippedItem Result;
		Result.ItemInstance = InventorySlot.ItemInstance;
		return Result;
	}

	// 기존 함수명 유지 (하위호환성)
	static FEquippedItem FromInventoryItem(const FInventorySlot& _BaseItem, int32 InQuantity = 1)
	{
		return FromInventorySlot(_BaseItem);
	}

	// FItemBase에서 직접 생성
	static FEquippedItem FromBase(const FItemBase& BaseItem, int32 Level = 1)
	{
		FEquippedItem Result;
		Result.ItemInstance = FItemInstance::FromBase(BaseItem);
		Result.ItemInstance.Level = Level;
		return Result;
	}

	// FEquippedItem에서 FInventorySlot로 변환 (그리드 위치는 기본값으로 설정)
	FInventorySlot ToInventorySlot(int32 GridX = 0, int32 GridY = 0) const;
};

inline FEquippedItem FInventorySlot::ToEquippItem() const
{
	FEquippedItem Result;
	Result.ItemInstance = ItemInstance;
	return Result;
}

inline FInventorySlot FEquippedItem::ToInventorySlot(int32 GridX, int32 GridY) const
{
	FInventorySlot Result;
	Result.ItemInstance = ItemInstance;
	Result.GridX = GridX;
	Result.GridY = GridY;
	return Result;
}

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
	
		// 단일 인덱스 접근을 위한 [] 연산자
	bool& operator[](int32 Index)
	{
		return Cells[Index];
	}

	const bool& operator[](int32 Index) const
	{
		return Cells[Index];
	}
};
