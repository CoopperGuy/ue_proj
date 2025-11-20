// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/ItemBase.h"  
#include "ItemSubsystem.generated.h"

class UItemWidget;
/**
 * 
 */
UCLASS()
class ARPG_API UItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    // USubsystem 
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    void LoadItemData();
	void LoadOptionData();

    const FItemBase& GetItemData(const FName& ItemID) const;
    
    FInventorySlot CreateInventoryInstance(const FName& ItemID, int32 Level = 1, bool bRandomStats = false);
    
	UItemWidget* CreateItemWidget(const FInventorySlot& Item);
    
private:
    UPROPERTY()
    UDataTable* ItemDataTable;
    
    UPROPERTY()
    FString ItemDataTablePath = TEXT("/Game/Datatable/DT_DiaitemTable.DT_DiaitemTable");
    
    UPROPERTY()
    UDataTable* OptionDataTable;
    
    UPROPERTY()
    FString OptionDataTablePath = TEXT("/Game/Datatable/DT_DiaitemTable.DT_DiaitemTable");

    UPROPERTY()
    mutable TMap<FName, FItemBase> ItemCache;
    
    UPROPERTY()
    mutable TMap<FName, FDiaItemOptionRow> OptionCache;

    void GenerateRandomStats(FItemInstance& Item, int32 Level);
	void GenerateItemOptions(FItemInstance& Item, int32 Level);


    //Efraimidis–Spirakis 를 통해 가중치 기반 표본 추출을 해봅시다.
	template<typename T>
    void PickupRandomValuesByWeight(const TArray<T>& Rows, TFunctionRef<float(const T&)> GetWight, int32 NumToPickupSize, TArray<T>& OutPicked)
    {
        if (Rows.Num() == 0 || NumToPickupSize <= 0)
            return;

		TArray<TPair<int, double>> CumulativeWeights;
        
        for (int32 i = 0; i < Rows.Num(); ++i)
        {
            double W = FMath::Max(GetWight(Rows[i]), 1e-6);
            double U = FMath::Clamp(FMath::FRand(), 1e-12, 1.f);
            double val = FMath::Exp(FMath::Loge(U) / W);
			CumulativeWeights.Add(TPair<int, double>(i, val));
        }

        if (CumulativeWeights.Num() <= 0)
            return;

		//큰 값을 우선으로 정렬
        CumulativeWeights.Sort([](const TPair<int, double>& A, const TPair<int, double>& B) {
			return A.Value > B.Value;
            });

		int32 PickupCount = FMath::Min(NumToPickupSize, CumulativeWeights.Num());
        for (int32 i = 0; i < PickupCount; ++i)
        {
            int32 Index = CumulativeWeights[i].Key;
            OutPicked.Add(Rows[Index]);
		}
    }
};
    