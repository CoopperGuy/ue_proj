#include "GAS/DiaMeleeTargeting.h"
#include "DiaBaseCharacter.h"
#include "Types/DiaGASSkillExtraData.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"

void DiaMeleeTargeting::GatherTargets(ADiaBaseCharacter& Attacker, const FGASMeleeData& Data,
    const FVector& Origin, const FRotator& Facing, TArray<ADiaBaseCharacter*>& OutTargets)
{
    OutTargets.Reset();
    UWorld* World = Attacker.GetWorld();
    if (!World || Data.AttackRange <= 0.f) return;

    const FVector Center = Origin + Facing.RotateVector(Data.AttackOffset);
    const FVector Forward = Facing.Vector().GetSafeNormal2D();
    FCollisionQueryParams Query(SCENE_QUERY_STAT(DiaMelee), false, &Attacker);
    TArray<FOverlapResult> Overlaps;
    World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(Data.AttackRange), Query);

    // Pawns cannot hide one another's hitboxes. Walls and solid props still block.
    FCollisionObjectQueryParams Obstructions;
    Obstructions.AddObjectTypesToQuery(ECC_WorldStatic);
    Obstructions.AddObjectTypesToQuery(ECC_WorldDynamic);
    for (const FOverlapResult& Hit : Overlaps)
    {
        ADiaBaseCharacter* Target = Cast<ADiaBaseCharacter>(Hit.GetActor());
        if (!Target || Target == &Attacker || Hit.GetComponent() != Target->GetCapsuleComponent()
            || OutTargets.Contains(Target)) continue;
        const bool bSameSide = (Attacker.ActorHasTag(TEXT("Actor.Monster")) && Target->ActorHasTag(TEXT("Actor.Monster")))
            || (Attacker.ActorHasTag(TEXT("Actor.Player")) && Target->ActorHasTag(TEXT("Actor.Player")));
        if (bSameSide) continue;

        const FVector Delta = Target->GetActorLocation() - Origin;
        const float Distance = Delta.Size2D();
        const float Angle = FMath::Acos(FMath::Clamp(FVector::DotProduct(Forward, Delta.GetSafeNormal2D()), -1.f, 1.f));
        const float CapsuleAngle = FMath::Asin(FMath::Clamp(Target->GetCapsuleComponent()->GetScaledCapsuleRadius()
            / FMath::Max(Distance, 1.f), 0.f, 1.f));
        if (Angle > FMath::DegreesToRadians(Data.AttackAngle * 0.5f) + CapsuleAngle) continue;

        FCollisionQueryParams VisibilityQuery = Query;
        VisibilityQuery.AddIgnoredActor(Target);
        TArray<FHitResult> WorldHits;
        World->LineTraceMultiByObjectType(WorldHits, Origin, Target->GetActorLocation(), Obstructions, VisibilityQuery);
        const bool bOccluded = WorldHits.ContainsByPredicate([](const FHitResult& WorldHit)
        {
            // Object queries also report overlap-only portal/room triggers.
            // Only solid geometry that blocks characters can stop a melee hit.
            return WorldHit.GetComponent()
                && WorldHit.GetComponent()->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block;
        });
        if (bOccluded) continue;
        OutTargets.Add(Target);
    }
    if (Data.bShowDebugShape)
    {
        DrawDebugSphere(World, Center, Data.AttackRange, 24, FColor::Red, false, 0.05f);
        for (float Sign : {-1.f, 1.f})
            DrawDebugLine(World, Origin, Origin + Forward.RotateAngleAxis(Data.AttackAngle * 0.5f * Sign, FVector::UpVector)
                * Data.AttackRange, FColor::Yellow, false, 0.05f);
        for (ADiaBaseCharacter* Target : OutTargets)
            DrawDebugLine(World, Origin, Target->GetActorLocation(), FColor::Green, false, 0.1f);
    }
}
