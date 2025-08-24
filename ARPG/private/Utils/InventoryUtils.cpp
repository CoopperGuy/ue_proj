#include "Utils/InventoryUtils.h"
#include "DiaComponent/UI/DiaInventoryComponent.h"
#include "Engine/Engine.h"
#include "System/ItemSubsystem.h"
#include "UI/Item/ItemWidget.h"

bool FInventoryUtils::CanPlaceItemAt(UDiaInventoryComponent* InventoryComponent, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
    if (!IsValid(InventoryComponent))
    {
        return false;
    }

    // 기본 범위 검사
    if (!IsValidGridPosition(InventoryComponent->GetGridWidth(), InventoryComponent->GetGridHeight(), 
                           ItemWidth, ItemHeight, PosX, PosY))
    {
        return false;
    }

    // 인벤토리 컴포넌트의 그리드에서 셀 점유 상태 확인
    const FGrid& Grid = InventoryComponent->GetInventoryGrid();
    
    for (int32 Y = PosY; Y < PosY + ItemHeight; ++Y)
    {
        for (int32 X = PosX; X < PosX + ItemWidth; ++X)
        {
            int32 CellIndex = Y * InventoryComponent->GetGridWidth() + X;
            if (CellIndex >= 0 && CellIndex < Grid.Cells.Num())
            {
                if (Grid.Cells[CellIndex]) // 셀이 이미 점유됨
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool FInventoryUtils::FindPlaceForItem(UDiaInventoryComponent* InventoryComponent, int32 ItemWidth, int32 ItemHeight, int32& OutPosX, int32& OutPosY)
{
    if (!IsValid(InventoryComponent))
    {
        return false;
    }

    const int32 GridWidth = InventoryComponent->GetGridWidth();
    const int32 GridHeight = InventoryComponent->GetGridHeight();

    // 좌상단부터 순차적으로 배치 가능한 위치 찾기
    for (int32 Y = 0; Y <= GridHeight - ItemHeight; ++Y)
    {
        for (int32 X = 0; X <= GridWidth - ItemWidth; ++X)
        {
            if (CanPlaceItemAt(InventoryComponent, ItemWidth, ItemHeight, X, Y))
            {
                OutPosX = X;
                OutPosY = Y;
                return true;
            }
        }
    }

    return false; // 배치할 수 있는 공간이 없음
}

bool FInventoryUtils::IsValidGridPosition(int32 GridWidth, int32 GridHeight, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY)
{
    // 음수 좌표 검사
    if (PosX < 0 || PosY < 0)
    {
        return false;
    }

    // 그리드 범위 초과 검사
    if (PosX + ItemWidth > GridWidth || PosY + ItemHeight > GridHeight)
    {
        return false;
    }

    // 아이템 크기 유효성 검사
    if (ItemWidth <= 0 || ItemHeight <= 0)
    {
        return false;
    }

    return true;
}

void FInventoryUtils::GetOccupiedCellIndices(int32 GridWidth, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY, TArray<int32>& OutIndices)
{
    OutIndices.Empty();

    for (int32 Y = PosY; Y < PosY + ItemHeight; ++Y)
    {
        for (int32 X = PosX; X < PosX + ItemWidth; ++X)
        {
            int32 CellIndex = Y * GridWidth + X;
            OutIndices.Add(CellIndex);
        }
    }
}

UItemWidget* FInventoryUtils::CreateItemWidget(const UObject* WorldContext, const FInventorySlot& ItemData)
{
    if (!IsValid(WorldContext))
    {
        UE_LOG(LogTemp, Warning, TEXT("FInventoryUtils::CreateItemWidget - WorldContext is null"));
        return nullptr;
    }

    UWorld* World = WorldContext->GetWorld();
    if (!IsValid(World))
    {
        UE_LOG(LogTemp, Warning, TEXT("FInventoryUtils::CreateItemWidget - World is null"));
        return nullptr;
    }

    UGameInstance* GameInstance = World->GetGameInstance();
    if (!IsValid(GameInstance))
    {
        UE_LOG(LogTemp, Warning, TEXT("FInventoryUtils::CreateItemWidget - GameInstance is null"));
        return nullptr;
    }

    UItemSubsystem* ItemSubsystem = GameInstance->GetSubsystem<UItemSubsystem>();
    if (!IsValid(ItemSubsystem))
    {
        UE_LOG(LogTemp, Warning, TEXT("FInventoryUtils::CreateItemWidget - ItemSubsystem is null"));
        return nullptr;
    }

    // ItemSubsystem에서 아이템 위젯을 생성한다.
    UItemWidget* ItemWidget = ItemSubsystem->CreateItemWidget(ItemData);
    if (!IsValid(ItemWidget))
    {
        UE_LOG(LogTemp, Warning, TEXT("FInventoryUtils::CreateItemWidget - Failed to create ItemWidget for item: %s"), 
               *ItemData.ItemInstance.BaseItem.ItemID.ToString());
        return nullptr;
    }

    UE_LOG(LogTemp, Log, TEXT("FInventoryUtils::CreateItemWidget - Successfully created ItemWidget for item: %s"), 
           *ItemData.ItemInstance.BaseItem.ItemID.ToString());

    return ItemWidget;
} 