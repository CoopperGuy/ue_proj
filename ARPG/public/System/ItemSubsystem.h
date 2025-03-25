// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/ItemBase.h"
#include "ItemSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ARPG_API UItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    // USubsystem 인터페이스
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    // 아이템 데이터 로드
    UFUNCTION(BlueprintCallable, Category = "Items")
    void LoadItemData();
    
    // 아이템 데이터 조회
    UFUNCTION(BlueprintCallable, Category = "Items")
    FItemBase* GetItemData(const FName& ItemID) const;
    
    // 인스턴스 아이템 생성
    UFUNCTION(BlueprintCallable, Category = "Items")
    FInventoryItem CreateItemInstance(const FName& ItemID, int32 Level = 1, bool bRandomStats = false);
    
    // 특정 조건의 아이템 검색
    UFUNCTION(BlueprintCallable, Category = "Items")
    TArray<FName> GetItemsByFilter(const FItemFilter& Filter);
    
private:
    UPROPERTY()
    UDataTable* ItemDataTable;
    
    // 경로 설정
    UPROPERTY()
    FString ItemDataTablePath = TEXT("/Game/Data/DT_Items");
    
    // 캐싱 콘테이너
    UPROPERTY()
    TMap<FName, FItemBase> ItemCache;
    
    // 디아블로 스타일 랜덤 속성 생성
    void GenerateRandomStats(FInventoryItem& Item, int32 Level);
};
