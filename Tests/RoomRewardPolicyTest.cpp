#include "../ARPG/public/Types/DiaRoomRewardPolicy.h"

using namespace DiaRoomRewardPolicy;
constexpr int Count(EDrop Type)
{
    int Result = 0;
    for (int Roll = 0; Roll < 100; ++Roll)
        if (FromPercentile(Roll) == Type) ++Result;
    return Result;
}
static_assert(Count(EDrop::None) == 50, "No-drop must be 50 percent");
static_assert(Count(EDrop::Gold) == 30, "Gold must be 30 percent");
static_assert(Count(EDrop::Heal) == 15, "Healing must be 15 percent");
static_assert(Count(EDrop::Equipment) == 5, "Equipment must be 5 percent");
static_assert(FromPercentile(-1) == EDrop::None && FromPercentile(100) == EDrop::None, "Reject out-of-range rolls");
static_assert(FromPercentile(49) == EDrop::None && FromPercentile(50) == EDrop::Gold, "Boundary 50");
static_assert(FromPercentile(79) == EDrop::Gold && FromPercentile(80) == EDrop::Heal, "Boundary 80");
static_assert(FromPercentile(94) == EDrop::Heal && FromPercentile(95) == EDrop::Equipment, "Boundary 95");
int main()
{
    int Total = 0;
    for (int Kind = 0; Kind < 4; ++Kind) Total += Count(static_cast<EDrop>(Kind));
    return Total == 100 ? 0 : 1;
}
