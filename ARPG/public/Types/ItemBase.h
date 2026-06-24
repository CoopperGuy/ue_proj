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

USTRUCT(BlueprintType)
struct ARPG_API FDiaItemStatOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StatTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Values = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemOptionScalingType ScalingType = EItemOptionScalingType::IOST_Flat;
};

USTRUCT(BlueprintType)
struct ARPG_API FItemBase : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID = "None";
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDiaItemStatOption> StatOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer PossibleItemOptionTags;
};

USTRUCT(BlueprintType)
struct ARPG_API FItemInstance
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID = "None";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FGuid InstanceID = FGuid();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Level = 1;
			
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bIsLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FDiaActualItemOption> PrefixOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FDiaActualItemOption> SuffixOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TMap<FGameplayTag, FDiaItemStatOption> ModifiedStats;

	FItemInstance()
		: InstanceID(FGuid::NewGuid())
	{
		PrefixOptions.Reserve(MAX_PREFIX_OPTIONS);
		SuffixOptions.Reserve(MAX_SUFFIX_OPTIONS);
	}

	bool IsValid() const { return !ItemID.IsNone() && Quantity > 0; }

	bool CheckPrefixOptionsSize() const { return PrefixOptions.Num() < MAX_PREFIX_OPTIONS; }
	bool CheckSuffixOptionsSize() const { return SuffixOptions.Num() < MAX_SUFFIX_OPTIONS; }

	float MakeScaledStatValue(float BaseValue) const
	{
		float LevelMultiplier = 1.0f + (Level - 1) * 0.1f;
		float QuantityMultiplier = 1.0f + (Quantity - 1) * 0.05f;

		return LevelMultiplier * QuantityMultiplier * BaseValue;
	}

	static FItemInstance FromDataTable(UDataTable* ItemTable, FName ItemID, int32 InQuantity = 1)
	{
		FItemInstance Result;
		
		if (ItemTable)
		{
			if (FItemBase* FoundItem = ItemTable->FindRow<FItemBase>(ItemID, TEXT("FItemInstance::FromDataTable")))
			{
				Result.ItemID = FoundItem->ItemID;
				Result.Quantity = InQuantity;
				for (const FDiaItemStatOption& StatOption : FoundItem->StatOptions)
				{
					FDiaItemStatOption NewStat = StatOption;
					NewStat.Values = Result.MakeScaledStatValue(StatOption.Values);
					Result.ModifiedStats.Add(StatOption.StatTag, NewStat);
				}
			}
		}
		
		return Result;
	}

	static FItemInstance FromBase(const FItemBase& ItemData, int32 InQuantity = 1)
	{
		FItemInstance Result;
		Result.Quantity = InQuantity;
		Result.ItemID = ItemData.ItemID;

		for(const auto& StatOption : ItemData.StatOptions)
		{
			FDiaItemStatOption NewStat = StatOption;
			NewStat.Values = Result.MakeScaledStatValue(StatOption.Values);
			Result.ModifiedStats.Add(StatOption.StatTag, NewStat);
		}
		return Result;
	}
};

// ?몃깽?좊━ ?щ’
USTRUCT(BlueprintType)
struct ARPG_API FInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FItemInstance ItemInstance;
	
	// 洹몃━???꾩튂 ?뺣낫
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridX = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridY = 0;

	bool IsEmpty() const { 
		if(ItemInstance.ItemID.IsNone())
		{
			return true;
		}
		return !ItemInstance.IsValid(); 
	}
	
	void Clear() 
	{ 
		ItemInstance = FItemInstance();
		GridX = GridY = 0;
	}

	static FInventorySlot FromBase(const FItemBase& ItemData, int32 InQuantity = 1)
	{
		FInventorySlot Result; 
		Result.ItemInstance = FItemInstance::FromBase(ItemData, InQuantity);
		return Result;
	}
	
	FEquippedItem ToEquippItem() const;
};

//?λ퉬??
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

	static FEquippedItem FromInventorySlot(const FInventorySlot& InventorySlot)
	{
		FEquippedItem Result;
		Result.ItemInstance = InventorySlot.ItemInstance;
		return Result;
	}

	static FEquippedItem FromInventoryItem(const FInventorySlot& InventorySlot, int32 InQuantity = 1)
	{
		return FromInventorySlot(InventorySlot);
	}

	static FEquippedItem FromBase(const FItemBase& ItemData, int32 Level = 1)
	{
		FEquippedItem Result;
		Result.ItemInstance = FItemInstance::FromBase(ItemData);
		Result.ItemInstance.Level = Level;
		return Result;
	}

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
struct FInventoryGridCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOccupied = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ItemInstanceID;
};

USTRUCT(BlueprintType)
struct ARPG_API FInventoryGrid
{
	GENERATED_BODY()

	FInventoryGrid() :Width(5), Height(5) 
	{
		Cells.SetNum(Width * Height);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FInventoryGridCell> Cells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Width;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Height;

	FInventoryGrid(int32 InWidth, int32 InHeight)
		: Width(InWidth), Height(InHeight)
	{
		Cells.SetNum(Width * Height);
	}

	FInventoryGridCell& operator()(int32 X, int32 Y)
	{
		return Cells[Y * Width + X];
	}

	const FInventoryGridCell& operator()(int32 X, int32 Y) const
	{
		return Cells[Y * Width + X];
	}

	FInventoryGridCell& operator[](int32 Index)
	{
		return Cells[Index];
	}

	const FInventoryGridCell& operator[](int32 Index) const
	{
		return Cells[Index];
	}
};
