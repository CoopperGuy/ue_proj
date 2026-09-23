#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"
#include "System/DiaSaveGame.h"
#include "System/DiaRewardSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Types/DiaRoomRewardPolicy.h"

namespace
{
    UDiaSaveGame* MakeFloorSave()
    {
        UDiaSaveGame* Save = NewObject<UDiaSaveGame>();
        Save->SaveVersion = 1;
        Save->bHasDungeonFloor = true;
        Save->FloorSize = FIntPoint(2, 1);
        Save->RoomLayout.SetNum(2);
        for (int32 Index = 0; Index < 2; ++Index)
        {
            FDiaRoomData& Room = Save->RoomLayout[Index];
            Room.RoomID = Index == 0 ? TEXT("Entrance") : TEXT("Room");
            Room.RoomGuid = FGuid::NewGuid();
            Room.RoomPosition = FIntPoint(Index, 0);
            Room.TileType = ETileType::Floor;
        }
        Save->StartRoom = Save->RoomLayout[0];
        return Save;
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaRoomRewardPolicyTest, "ARPG.RoomRewards.ProbabilityBoundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDiaRoomRewardPolicyTest::RunTest(const FString& Parameters)
{
    int32 Counts[4] = {};
    for (int32 Roll = 0; Roll < 100; ++Roll) ++Counts[static_cast<int32>(DiaRoomRewardPolicy::FromPercentile(Roll))];
    TestEqual(TEXT("No extra reward"), Counts[0], 50);
    TestEqual(TEXT("Gold"), Counts[1], 30);
    TestEqual(TEXT("Heal"), Counts[2], 15);
    TestEqual(TEXT("Equipment"), Counts[3], 5);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaFloorSnapshotValidationTest, "ARPG.RoomRewards.SnapshotValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDiaFloorSnapshotValidationTest::RunTest(const FString& Parameters)
{
    UDiaSaveGame* Save = MakeFloorSave();
    TestTrue(TEXT("Valid layout"), Save->IsFloorSnapshotValid());
    const FGuid Room = Save->RoomLayout[1].RoomGuid;
    FDiaRoomRewardState NoDrop;
    NoDrop.bResolved = true;
    Save->RoomRewards.Add(Room, NoDrop);
    Save->ClearedRooms.Add(Room);
    TestTrue(TEXT("Resolved no-drop is a valid persisted outcome"), Save->IsFloorSnapshotValid());
    Save->RoomRewards[Room].bClaimed = true;
    TestFalse(TEXT("Cannot claim an empty outcome"), Save->IsFloorSnapshotValid());
    Save->RoomRewards[Room].bClaimed = false;
    Save->RoomRewards[Room].Kind = EDiaRoomRewardKind::Boss;
    TestFalse(TEXT("Boss cannot persist an empty choice"), Save->IsFloorSnapshotValid());
    Save->RoomRewards.Reset();
    Save->RoomLayout[1].RoomGuid = Save->StartRoom.RoomGuid;
    TestFalse(TEXT("Duplicate room identity rejected"), Save->IsFloorSnapshotValid());
    Save->RoomLayout[1].RoomGuid = Room;
    Save->FloorSize = FIntPoint(50000, 50000);
    TestFalse(TEXT("Oversized dimensions rejected before multiplication"), Save->IsFloorSnapshotValid());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaRoomRewardSaveRoundTripTest, "ARPG.RoomRewards.SaveRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDiaRoomRewardSaveRoundTripTest::RunTest(const FString& Parameters)
{
    UDiaSaveGame* Save = MakeFloorSave();
    const FGuid Room = Save->RoomLayout[1].RoomGuid;
    FRewardData Item;
    Item.RewardType = ERewardType::Item;
    Item.ItemId = TEXT("SavedTestEquipment");
    Item.SourceRoomGuid = Room;
    Item.SourceOptionIndex = 0;
    Item.bHasPreparedItem = true;
    Item.PreparedItem.ItemInstance.ItemID = Item.ItemId;
    Item.PreparedItem.ItemInstance.Level = 7;
    const FGuid Instance = Item.PreparedItem.ItemInstance.InstanceID;
    FDiaRoomRewardState State;
    State.bResolved = true;
    State.Options.Add(Item);
    Save->RoomRewards.Add(Room, State);
    Save->ClearedRooms.Add(Room);
    Save->Gold = 123;
    Save->Health = 31.f;
    Save->Mana = 17.f;
    Save->Experience = 42.f;
    FDiaSavedSkill Skill;
    Skill.SkillID = 10;
    Skill.Level = 3;
    Skill.OwnedVariants.Add(11);
    Skill.AppliedVariants.Add(11);
    Save->Skills.Add(Skill);
    Save->QuickSlots = {10, -1, -1, -1, -1};
    TArray<uint8> Bytes;
    if (!TestTrue(TEXT("Serialize snapshot"), UGameplayStatics::SaveGameToMemory(Save, Bytes))) return false;
    UDiaSaveGame* Loaded = Cast<UDiaSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if (!TestNotNull(TEXT("Deserialize snapshot"), Loaded)) return false;
    TestTrue(TEXT("Loaded snapshot valid"), Loaded->IsFloorSnapshotValid());
    const FDiaRoomRewardState* Restored = Loaded->RoomRewards.Find(Room);
    if (!TestNotNull(TEXT("Stable room key"), Restored) || Restored->Options.Num() != 1) return false;
    TestTrue(TEXT("Unclaimed stays unclaimed"), !Restored->bClaimed);
    TestEqual(TEXT("Item identity does not reroll"), Restored->Options[0].PreparedItem.ItemInstance.InstanceID, Instance);
    TestEqual(TEXT("Prepared item level"), Restored->Options[0].PreparedItem.ItemInstance.Level, 7);
    TestEqual(TEXT("Gold"), Loaded->Gold, 123);
    TestEqual(TEXT("Health"), Loaded->Health, 31.f);
    TestEqual(TEXT("Experience"), Loaded->Experience, 42.f);
    TestTrue(TEXT("Cleared room persists"), Loaded->ClearedRooms.Contains(Room));
    if (!TestEqual(TEXT("Skill count"), Loaded->Skills.Num(), 1)) return false;
    TestTrue(TEXT("Applied variant persists"), Loaded->Skills[0].AppliedVariants.Contains(11));
    TestEqual(TEXT("Quick slot"), Loaded->QuickSlots[0], 10);

    UGameInstance* TestGameInstance = NewObject<UGameInstance>();
    UDiaRewardSubsystem* Rewards = NewObject<UDiaRewardSubsystem>(TestGameInstance);
    Rewards->RestoreFloor(Loaded->RoomRewards);
    TestFalse(TEXT("Missing controller cannot consume reward"), Rewards->TryClaimRoomReward(Room, 0, nullptr));
    TestTrue(TEXT("Failed claim remains available"), !Rewards->FindRoomReward(Room)->bClaimed);
    Loaded->RoomRewards[Room].bClaimed = true;
    Bytes.Reset();
    UGameplayStatics::SaveGameToMemory(Loaded, Bytes);
    UDiaSaveGame* ClaimedSave = Cast<UDiaSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if (!TestNotNull(TEXT("Claimed save"), ClaimedSave)) return false;
    Rewards->RestoreFloor(ClaimedSave->RoomRewards);
    TestTrue(TEXT("Claimed state persists"), Rewards->FindRoomReward(Room)->bClaimed);
    TestFalse(TEXT("Claimed reward cannot be consumed again"), Rewards->TryClaimRoomReward(Room, 0, nullptr));
    Rewards->ResetFloor();
    TestTrue(TEXT("New floor resets the ledger"), Rewards->GetRoomRewards().IsEmpty());
    return true;
}
#endif
