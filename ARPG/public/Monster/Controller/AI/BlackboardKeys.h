#pragma once

#include "CoreMinimal.h"

namespace BlackboardKeys
{
    namespace Monster
    {
        static const FName TargetActor = TEXT("TargetActor");
        static const FName PatrolPoint = TEXT("PatrolPoint");
        static const FName LastKnownLocation = TEXT("LastKnownLocation");
        static const FName SaveTargetLocation = TEXT("SaveTargetLocation");
        static const FName IsInCombat = TEXT("IsInCombat");
        static const FName HomeLocation = TEXT("HomeLocation");
        static const FName CurrentState = TEXT("CurrentState");
        static const FName CanSeePlayer = TEXT("CanSeePlayer");
        static const FName AlertLevel = TEXT("AlertLevel");
        static const FName LOS = TEXT("HasLineOfSight");
		static const FName ActionRequest = TEXT("ActionRequest");
    }
    
    namespace Player
    {
        static const FName PlayerRef = TEXT("PlayerRef");
        static const FName IsPlayerInSight = TEXT("IsPlayerInSight");
        static const FName PlayerLastKnownLocation = TEXT("PlayerLastKnownLocation");
    }
    
    namespace Combat
    {
        static const FName AttackTarget = TEXT("AttackTarget");
        static const FName AttackRange = TEXT("AttackRange");
        static const FName LastAttackTime = TEXT("LastAttackTime");
        static const FName IsAttacking = TEXT("IsAttacking");
    }
    
    namespace Patrol
    {
        static const FName PatrolPoints = TEXT("PatrolPoints");
        static const FName CurrentPatrolIndex = TEXT("CurrentPatrolIndex");
        static const FName PatrolWaitTime = TEXT("PatrolWaitTime");
        static const FName IsPatrolling = TEXT("IsPatrolling");
    }
}