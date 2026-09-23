#pragma once

#include "CoreMinimal.h"

class ADiaBaseCharacter;
struct FGASMeleeData;

namespace DiaMeleeTargeting
{
    // Capsule contact, horizontal cone and world occlusion share one definition
    // for step-based and retained legacy attacks.
    void GatherTargets(ADiaBaseCharacter& Attacker, const FGASMeleeData& Data,
        const FVector& Origin, const FRotator& Facing, TArray<ADiaBaseCharacter*>& OutTargets);
}
