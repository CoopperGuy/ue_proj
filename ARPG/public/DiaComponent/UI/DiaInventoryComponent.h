// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ItemBase.h"

#include "DiaInventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ARPG_API UDiaInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDiaInventoryComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    // 아이템을 그리드에 추가 시도
    bool TryAddItem(const FInventoryItem& ItemData, int32 PosX, int32 PosY);
    
    // 그리드 위치에 아이템 배치 가능 여부 확인
    bool CanPlaceItemAt(int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);	

private:
    // 아이템 데이터 저장
    UPROPERTY()
    TArray<FInventoryItem> Items;

    // 그리드 크기 설정
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridWidth = 10;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 GridHeight = 5;
    
    // 그리드 상태 저장 (true = 점유됨)
    UPROPERTY()
    FGrid InventoryGrid;
    
};
