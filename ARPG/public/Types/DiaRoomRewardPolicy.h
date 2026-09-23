#pragma once

// Engine-independent policy so the actual production thresholds can be tested
// without starting the editor. The caller supplies a uniform integer in [0,99].
namespace DiaRoomRewardPolicy
{
    enum class EDrop { None, Gold, Heal, Equipment };
    constexpr EDrop FromPercentile(int Roll)
    {
        return Roll < 0 || Roll >= 100 ? EDrop::None
            : Roll < 50 ? EDrop::None
            : Roll < 80 ? EDrop::Gold
            : Roll < 95 ? EDrop::Heal : EDrop::Equipment;
    }
}
