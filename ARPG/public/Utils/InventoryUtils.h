#pragma once

#include "CoreMinimal.h"
#include "Types/ItemBase.h"

class UDiaInventoryComponent;
class UItemWidget;

/**
 * 인벤토리 관련 공통 유틸리티 함수들
 * 순환 참조 없이 여러 클래스에서 공통으로 사용할 수 있는 정적 함수들 제공
 */
class ARPG_API FInventoryUtils
{
public:
    /**
     * 아이템을 특정 위치에 배치할 수 있는지 검증
     * @param InventoryComponent 검증할 인벤토리 컴포넌트
     * @param ItemWidth 아이템 너비 (그리드 단위)
     * @param ItemHeight 아이템 높이 (그리드 단위)
     * @param PosX 배치할 X 위치
     * @param PosY 배치할 Y 위치
     * @return 배치 가능하면 true
     */
    static bool CanPlaceItemAt(UDiaInventoryComponent* InventoryComponent, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);

    /**
     * 아이템을 배치할 수 있는 빈 공간 찾기
     * @param InventoryComponent 검색할 인벤토리 컴포넌트
     * @param ItemWidth 아이템 너비 (그리드 단위)
     * @param ItemHeight 아이템 높이 (그리드 단위)
     * @param OutPosX 찾은 위치의 X 좌표 (출력)
     * @param OutPosY 찾은 위치의 Y 좌표 (출력)
     * @return 빈 공간을 찾았으면 true
     */
    static bool FindPlaceForItem(UDiaInventoryComponent* InventoryComponent, int32 ItemWidth, int32 ItemHeight, int32& OutPosX, int32& OutPosY);

    /**
     * 그리드 범위 내 유효성 검사
     * @param GridWidth 그리드 너비
     * @param GridHeight 그리드 높이
     * @param ItemWidth 아이템 너비
     * @param ItemHeight 아이템 높이
     * @param PosX X 위치
     * @param PosY Y 위치
     * @return 유효한 범위면 true
     */
    static bool IsValidGridPosition(int32 GridWidth, int32 GridHeight, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY);

    /**
     * 아이템이 차지하는 그리드 셀들의 인덱스 계산
     * @param GridWidth 그리드 너비
     * @param ItemWidth 아이템 너비
     * @param ItemHeight 아이템 높이
     * @param PosX X 위치
     * @param PosY Y 위치
     * @param OutIndices 결과 인덱스들 (출력)
     */
    static void GetOccupiedCellIndices(int32 GridWidth, int32 ItemWidth, int32 ItemHeight, int32 PosX, int32 PosY, TArray<int32>& OutIndices);

    /**
     * ItemSubsystem을 통해 아이템 위젯 생성
     * @param WorldContext 월드 컨텍스트를 제공하는 UObject (GameInstance 접근용)
     * @param ItemData 생성할 아이템 데이터
     * @return 생성된 ItemWidget (실패 시 nullptr)
     */
    static UItemWidget* CreateItemWidget(const UObject* WorldContext, const FInventorySlot* ItemData);
}; 