// Fill out your copyright notice in the Description page of Project Settings.


#include "System/DiaSaveGame.h"





bool UDiaSaveGame::IsFloorSnapshotValid() const
{
    if (!bHasDungeonFloor || FloorSize.X < 1 || FloorSize.Y < 1 || FloorSize.X > 50 || FloorSize.Y > 50
        || RoomLayout.Num() != FloorSize.X * FloorSize.Y || !StartRoom.RoomGuid.IsValid()) return false;
    TSet<FGuid> IDs;
    bool bFoundStart = false;
    for (int32 Index = 0; Index < RoomLayout.Num(); ++Index)
    {
        const FDiaRoomData& Room = RoomLayout[Index];
        if (Room.RoomID.IsNone()) continue;
        if (!Room.RoomGuid.IsValid() || Room.RoomPosition.X < 0 || Room.RoomPosition.Y < 0
            || Room.RoomSize.X < 1 || Room.RoomSize.Y < 1
            || Room.RoomPosition.X + Room.RoomSize.X > FloorSize.X
            || Room.RoomPosition.Y + Room.RoomSize.Y > FloorSize.Y) return false;
        const int32 AnchorIndex = Room.RoomPosition.Y * FloorSize.X + Room.RoomPosition.X;
        const FDiaRoomData& Anchor = RoomLayout[AnchorIndex];
        if (!Anchor.bIsAnchor || Anchor.RoomGuid != Room.RoomGuid || Anchor.RoomID != Room.RoomID
            || Anchor.RoomSize != Room.RoomSize || Room.bIsAnchor != (Index == AnchorIndex)) return false;
        if (!Room.bIsAnchor) continue;
        if (IDs.Contains(Room.RoomGuid)) return false;
        IDs.Add(Room.RoomGuid);
        if (Room.RoomGuid == StartRoom.RoomGuid)
            bFoundStart = Room.RoomPosition == StartRoom.RoomPosition && Room.RoomID == StartRoom.RoomID;
    }
    if (!bFoundStart) return false;
    for (const FGuid& Room : ClearedRooms) if (!IDs.Contains(Room)) return false;
    for (const auto& Entry : RoomRewards)
    {
        const FDiaRoomRewardState& State = Entry.Value;
        if (!IDs.Contains(Entry.Key) || !State.bResolved) return false;
        const int32 Expected = State.Kind == EDiaRoomRewardKind::Boss ? 2 : 1;
        if (State.Options.Num() > Expected || (State.Kind != EDiaRoomRewardKind::Normal && State.Options.Num() != Expected)) return false;
        if (State.bClaimed && State.Options.IsEmpty()) return false;
        for (int32 Index = 0; Index < State.Options.Num(); ++Index)
        {
            const FRewardData& Option = State.Options[Index];
            if (Option.SourceRoomGuid != Entry.Key || Option.SourceOptionIndex != Index || Option.RewardType == ERewardType::None) return false;
            if (Option.RewardType == ERewardType::Item && (!Option.bHasPreparedItem || Option.PreparedItem.IsEmpty()
                || !Option.PreparedItem.ItemInstance.InstanceID.IsValid() || Option.PreparedItem.ItemInstance.ItemID != Option.ItemId)) return false;
        }
    }
    return true;
}
