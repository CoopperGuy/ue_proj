#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GAS/DiaGameplayAbility.h"
#include "GAS/DiaGASHelper.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "DiaBaseCharacter.h"
#include "GAS/DiaAttributeSet.h"
#include "Monster/DiaMonster.h"
#include "Monster/Controller/DiaAIController.h"
#include "System/MonsterManager.h"
#include "Skill/DiaSkillActor.h"
#include "Map/DiaRoomBase.h"
#include "Tests/AutomationEditorCommon.h"

// Opens a disposable PIE session in the current dungeon map and closes it after.
// Uses the real monster pool, perception, BT, montage and GAS damage. No saves.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaMonsterCombatPIETest, "ARPG.Combat.MonsterAttacksPIE",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiaMonsterPrecisionPIETest, "ARPG.Combat.MonsterAttackPrecisionPIE",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
struct FCombatCase
{
    FName ID;
    float Distance;
    bool bStaleRequest = false;
    bool bCrowd = false;
    bool bPhaseTwo = false;
    bool bMovingTarget = false;
};

class FMonsterCombatCommand : public IAutomationLatentCommand
{
public:
    FMonsterCombatCommand(FAutomationTestBase* InTest, UWorld* InWorld, ADiaBaseCharacter* InPlayer)
        : Test(InTest), World(InWorld), Player(InPlayer), Center(InPlayer->GetActorLocation())
    {
        UAbilitySystemComponent* ASC = InPlayer->GetAbilitySystemComponent();
        Test->TestTrue(TEXT("Player effects retain their source ASC"),
            ASC->MakeEffectContext().GetInstigatorAbilitySystemComponent() == ASC);
        OriginalHealth = ASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
        OriginalMaxHealth = ASC->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute());
        ASC->SetNumericAttributeBase(UDiaAttributeSet::GetMaxHealthAttribute(), 10000.f);
        ASC->SetNumericAttributeBase(UDiaAttributeSet::GetHealthAttribute(), 10000.f);
        Cases = {{"Raptor_01", 600}, {"Raptor_02", 600}, {"Zombie_01", 600}, {"Zombie_02", 600},
            {"Bomber_01", 600}, {"BladeGuard_01", 600}, {"Spitter_01", 750},
            {"Charger_01", 550}, {"Warden_Boss", 600}, {"DualBlade_Boss", 550},
            {"BladeGuard_01", 210, true}, {"BladeGuard_01", 140, false, true},
            {"Warden_Boss", 250, false, false, true}, {"Warden_Boss", 650, false, false, true},
            {"Charger_01", 550, false, false, false, true}};
        WallStarted = FPlatformTime::Seconds();
    }

    virtual bool Update() override
    {
        if (!World.IsValid() || !Player.IsValid())
        {
            Test->AddError(TEXT("PIE ended before monster combat verification completed."));
            return true;
        }
        UMonsterManager* Manager = World->GetGameInstance()->GetSubsystem<UMonsterManager>();
        UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
        if (Index >= Cases.Num() || FPlatformTime::Seconds() - WallStarted > 420)
        {
            if (Index < Cases.Num()) Test->AddError(TEXT("Monster combat suite timed out."));
            Cleanup(Manager);
            PlayerASC->SetNumericAttributeBase(UDiaAttributeSet::GetMaxHealthAttribute(), OriginalMaxHealth);
            PlayerASC->SetNumericAttributeBase(UDiaAttributeSet::GetHealthAttribute(), OriginalHealth);
            return true;
        }
        const FCombatCase& Case = Cases[Index];
        if (!Monster.IsValid())
        {
            Player->SetActorLocation(Center, false, nullptr, ETeleportType::TeleportPhysics);
            Monster = Manager->SpawnMonster(World.Get(), Case.ID, Center - FVector(Case.Distance, 0, 0));
            if (!Monster.IsValid())
            {
                Test->AddError(FString::Printf(TEXT("Could not spawn %s"), *Case.ID.ToString()));
                ++Index;
                return false;
            }
            // Start facing away: perception and rotation tasks must acquire/aim.
            Monster->SetActorRotation(FRotator(0, 180, 0));
            UAbilitySystemComponent* MonsterASC = Monster->GetAbilitySystemComponent();
            Test->TestTrue(Case.ID.ToString() + TEXT(" effects retain their source ASC"),
                MonsterASC->MakeEffectContext().GetInstigatorAbilitySystemComponent() == MonsterASC);
            if (Case.bPhaseTwo)
            {
                UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent();
                ASC->SetNumericAttributeBase(UDiaAttributeSet::GetHealthAttribute(),
                    ASC->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute()) * 0.4f);
                Monster->OnRecieveDamage(0.f);
            }
            if (Case.bStaleRequest)
                Cast<ADiaAIController>(Monster->GetController())->GetBlackboardComponent()->SetValueAsInt(TEXT("ActionRequest"), 999999);
            if (Case.bCrowd)
            {
                for (float Side : {-120.f, 120.f})
                {
                    ADiaMonster* Ally = Manager->SpawnMonster(World.Get(), TEXT("BladeGuard_01"), Center + FVector(-140, Side, 0));
                    if (Ally)
                    {
                        Ally->DeactivateAI();
                        Blockers.Add(Ally);
                    }
                }
            }
            Started = World->GetTimeSeconds();
            LastHealth = PlayerASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
            BeforeHealth = LastHealth;
            HitEvents = 0;
            bSawMontage = false;
            AttackDrift = 0.f;
            PreviousMontage.Reset();
            return false;
        }
        const float Health = PlayerASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
        if (Health < LastHealth) ++HitEvents;
        LastHealth = Health;
        UAnimMontage* Montage = Monster->GetMesh()->GetAnimInstance()
            ? Monster->GetMesh()->GetAnimInstance()->GetCurrentActiveMontage() : nullptr;
        if (Montage)
        {
            bSawMontage = true;
            const FString Name = Montage->GetName();
            const bool bCharge = Name.Contains(TEXT("Dash")) || Name.Contains(TEXT("Rush")) || Name.Contains(TEXT("Charge"));
            if (!bCharge && Montage == PreviousMontage.Get())
                AttackDrift += FVector::Dist2D(PreviousLocation, Monster->GetActorLocation());
        }
        PreviousMontage = Montage;
        PreviousLocation = Monster->GetActorLocation();
        const float Elapsed = World->GetTimeSeconds() - Started;
        if (Case.bMovingTarget)
        {
            const float Side = Elapsed < 3.f ? 180.f * FMath::Sin(Elapsed * 2.f) : 0.f;
            Player->SetActorLocation(Center + FVector(0, Side, 0), true);
        }
        if ((HitEvents >= 2 && Elapsed >= 6.f) || Elapsed >= 22.f)
        {
            const FString Label = FString::Printf(TEXT("%s stale=%d crowd=%d phase2=%d moving=%d"),
                *Case.ID.ToString(), Case.bStaleRequest, Case.bCrowd, Case.bPhaseTwo, Case.bMovingTarget);
            Test->TestTrue(Label + TEXT(" deals repeated damage through AI"), HitEvents >= 2);
            Test->TestTrue(Label + TEXT(" plays attack montage"), bSawMontage);
            Test->TestTrue(Label + TEXT(" stays planted through non-charge montages"), AttackDrift < 3.f);
            Test->AddInfo(FString::Printf(TEXT("MONSTER_COMBAT %s hits=%d damage=%.1f time=%.1f montage=%d"),
                *Label, HitEvents, BeforeHealth - Health, Elapsed, bSawMontage));
            UE_LOG(LogTemp, Display, TEXT("MONSTER_COMBAT %s hits=%d damage=%.1f time=%.1f montage=%d distance=%.1f"),
                *Label, HitEvents, BeforeHealth - Health, Elapsed, bSawMontage,
                FVector::Dist(Monster->GetActorLocation(), Player->GetActorLocation()));
            Cleanup(Manager);
            ++Index;
        }
        return false;
    }

private:
    void Cleanup(UMonsterManager* Manager)
    {
        if (Monster.IsValid())
        {
            Monster->GetAbilitySystemComponent()->CancelAllAbilities();
            for (TActorIterator<ADiaSkillActor> It(World.Get()); It; ++It)
                if (It->GetOwner() == Monster.Get()) It->Destroy();
            Manager->DespawnMonster(Monster.Get());
            Monster.Reset();
        }
        for (const TWeakObjectPtr<ADiaMonster>& Ally : Blockers)
            if (Ally.IsValid()) Manager->DespawnMonster(Ally.Get());
        Blockers.Empty();
    }
    FAutomationTestBase* Test;
    TWeakObjectPtr<UWorld> World;
    TWeakObjectPtr<ADiaBaseCharacter> Player;
    TWeakObjectPtr<ADiaMonster> Monster;
    TArray<TWeakObjectPtr<ADiaMonster>> Blockers;
    TArray<FCombatCase> Cases;
    FVector Center;
    int32 Index = 0, HitEvents = 0;
    float Started = 0, LastHealth = 0, BeforeHealth = 0;
    float OriginalHealth = 0, OriginalMaxHealth = 0;
    double WallStarted = 0;
    bool bSawMontage = false;
    float AttackDrift = 0.f;
    FVector PreviousLocation;
    TWeakObjectPtr<UAnimMontage> PreviousMontage;
};

enum class EPrecisionAction { None, Dodge, EnterDuringHit, EnterAfterHit, Cancel, Wall, Trigger, ChaseRecovery, Charge };
struct FPrecisionCase
{
    const TCHAR* Label;
    FName MonsterID;
    int32 SkillID;
    FVector Offset;
    int32 ExpectedHits;
    EPrecisionAction Action = EPrecisionAction::None;
};

class FMonsterPrecisionCommand : public IAutomationLatentCommand
{
public:
    FMonsterPrecisionCommand(FAutomationTestBase* InTest, UWorld* InWorld, ADiaBaseCharacter* InPlayer)
        : Test(InTest), World(InWorld), Player(InPlayer), Center(InPlayer->GetActorLocation() - FVector(200, 0, 0))
    {
        auto* ASC = Player->GetAbilitySystemComponent();
        OldHealth = ASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
        OldMaxHealth = ASC->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute());
        ASC->SetNumericAttributeBase(UDiaAttributeSet::GetMaxHealthAttribute(), 10000);
        ASC->SetNumericAttributeBase(UDiaAttributeSet::GetHealthAttribute(), 10000);
        Cases = {
            {TEXT("front-single-hit"), "BladeGuard_01", 3101, {140,0,0}, 1},
            {TEXT("behind-misses"), "BladeGuard_01", 3101, {-140,0,0}, 0},
            {TEXT("side-misses"), "BladeGuard_01", 3101, {0,180,0}, 0},
            {TEXT("out-of-range-misses"), "BladeGuard_01", 3101, {310,0,0}, 0},
            {TEXT("wall-blocks"), "BladeGuard_01", 3101, {140,0,0}, 0, EPrecisionAction::Wall},
            {TEXT("trigger-does-not-block"), "BladeGuard_01", 3101, {140,0,0}, 1, EPrecisionAction::Trigger},
            {TEXT("dodge-windup"), "BladeGuard_01", 3101, {140,0,0}, 0, EPrecisionAction::Dodge},
            {TEXT("enter-active-window"), "BladeGuard_01", 3101, {310,0,0}, 1, EPrecisionAction::EnterDuringHit},
            {TEXT("enter-recovery-misses"), "BladeGuard_01", 3101, {310,0,0}, 0, EPrecisionAction::EnterAfterHit},
            {TEXT("interrupt-no-late-hit"), "BladeGuard_01", 3101, {140,0,0}, 0, EPrecisionAction::Cancel},
            {TEXT("chase-only-after-recovery"), "BladeGuard_01", 3101, {140,0,0}, 0, EPrecisionAction::ChaseRecovery},
            {TEXT("zombie-contact"), "Zombie_01", 3001, {140,0,0}, 1},
            {TEXT("zombie-dodge"), "Zombie_01", 3001, {140,0,0}, 0, EPrecisionAction::Dodge},
            {TEXT("raptor-contact"), "Raptor_01", 3003, {190,0,0}, 1},
            {TEXT("boss-two-swings"), "Warden_Boss", 3201, {230,0,0}, 2},
            {TEXT("charge-still-moves"), "Charger_01", 3103, {-450,0,0}, 1, EPrecisionAction::Charge}
        };
    }

    virtual bool Update() override
    {
        if (!World.IsValid() || !Player.IsValid()) { Test->AddError(TEXT("Precision PIE lost world/player")); return true; }
        UMonsterManager* Manager = World->GetGameInstance()->GetSubsystem<UMonsterManager>();
        auto* PlayerASC = Player->GetAbilitySystemComponent();
        if (Index >= Cases.Num())
        {
            PlayerASC->SetNumericAttributeBase(UDiaAttributeSet::GetMaxHealthAttribute(), OldMaxHealth);
            PlayerASC->SetNumericAttributeBase(UDiaAttributeSet::GetHealthAttribute(), OldHealth);
            return true;
        }
        const FPrecisionCase& Case = Cases[Index];
        if (!Monster.IsValid())
        {
            Player->SetActorLocation(Center + Case.Offset, false, nullptr, ETeleportType::TeleportPhysics);
            Monster = Manager->SpawnMonster(World.Get(), Case.MonsterID, Center);
            if (!Monster.IsValid()) { Test->AddError(TEXT("Precision spawn failed")); ++Index; return false; }
            Monster->DeactivateAI();
            Monster->SetActorRotation(FRotator(0, Case.Action == EPrecisionAction::Charge ? 180.f : 0.f, 0));
            auto* ASC = Monster->GetAbilitySystemComponent();
            ASC->CancelAllAbilities();
            ASC->RemoveActiveEffects(FGameplayEffectQuery());
            FGameplayAbilitySpec* Spec = UDiaGASHelper::GetAbilitySpecBySkillID(ASC, Case.SkillID);
            UDiaGameplayAbility* Ability = Spec ? Cast<UDiaGameplayAbility>(Spec->GetPrimaryInstance()) : nullptr;
            if (!Ability) { Test->AddError(TEXT("Precision skill missing")); Cleanup(Manager); ++Index; return false; }
            CastTime = Ability->GetSkillData().CastTime;
            if (Case.Action == EPrecisionAction::Wall || Case.Action == EPrecisionAction::Trigger)
            {
                Wall = World->SpawnActor<AActor>();
                UBoxComponent* Box = NewObject<UBoxComponent>(Wall.Get());
                Wall->SetRootComponent(Box);
                Box->SetBoxExtent(FVector(8,100,100));
                Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                Box->SetCollisionObjectType(ECC_WorldStatic);
                Box->SetCollisionResponseToAllChannels(ECR_Block);
                if (Case.Action == EPrecisionAction::Trigger) Box->SetCollisionResponseToAllChannels(ECR_Overlap);
                Box->RegisterComponent();
                Wall->SetActorLocation(Center + FVector(70,0,0));
            }
            Started = World->GetTimeSeconds();
            LastHealth = PlayerASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
            HitEvents = 0; EarlyHits = 0; Drift = 0; ActionDone = false; SawLock = false; Recovered = false;
            SpawnLocation = Monster->GetActorLocation();
            Test->TestTrue(FString(Case.Label) + TEXT(" activated"), ASC->TryActivateAbility(Spec->Handle));
            return false;
        }
        const float Elapsed = World->GetTimeSeconds() - Started;
        if (!ActionDone)
        {
            if ((Case.Action == EPrecisionAction::Dodge || Case.Action == EPrecisionAction::ChaseRecovery)
                && Elapsed >= FMath::Max(0.05f, CastTime - 0.18f))
            {
                Player->SetActorLocation(Center + FVector(0,360,0), false, nullptr, ETeleportType::TeleportPhysics);
                if (Case.Action == EPrecisionAction::ChaseRecovery) Monster->ActivateAI();
                ActionDone = true;
            }
            if ((Case.Action == EPrecisionAction::EnterDuringHit && Elapsed >= CastTime + 0.06f)
                || (Case.Action == EPrecisionAction::EnterAfterHit && Elapsed >= CastTime + 0.32f))
            {
                Player->SetActorLocation(Center + FVector(140,0,0), false, nullptr, ETeleportType::TeleportPhysics);
                ActionDone = true;
            }
            if (Case.Action == EPrecisionAction::Cancel && Elapsed >= CastTime * 0.5f)
            {
                Monster->GetAbilitySystemComponent()->CancelAllAbilities();
                ActionDone = true;
            }
        }
        const float Health = PlayerASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
        if (Health < LastHealth)
        {
            ++HitEvents;
            if (Elapsed + 0.03f < CastTime) ++EarlyHits;
        }
        LastHealth = Health;
        if (Monster->IsAttackInProgress())
        {
            SawLock = true;
            if (!Recovered && Case.Action != EPrecisionAction::Charge)
                Drift = FMath::Max(Drift, FVector::Dist2D(SpawnLocation, Monster->GetActorLocation()));
        }
        else if (SawLock && !Recovered)
        {
            Recovered = true;
            RecoveryLocation = Monster->GetActorLocation();
        }
        // Stop before a second chase attack can hit; the first swing must miss.
        const bool bChaseResumed = Recovered && Case.Action == EPrecisionAction::ChaseRecovery
            && FVector::Dist2D(RecoveryLocation, Monster->GetActorLocation()) > 35.f;
        if (bChaseResumed || Elapsed >= 3.6f)
        {
            const FString Label(Case.Label);
            Test->TestEqual(Label + TEXT(" hit count"), HitEvents, Case.ExpectedHits);
            Test->TestEqual(Label + TEXT(" no early damage"), EarlyHits, 0);
            Test->TestTrue(Label + TEXT(" released attack lock"), Recovered);
            Test->TestTrue(Label + TEXT(" no pursuit during attack"), Drift < 3.f);
            if (Case.Action == EPrecisionAction::ChaseRecovery)
                Test->TestTrue(Label + TEXT(" resumed chase"), bChaseResumed);
            if (Case.Action == EPrecisionAction::Charge)
                Test->TestTrue(Label + TEXT(" scripted charge moved"), FVector::Dist2D(SpawnLocation, Monster->GetActorLocation()) > 100.f);
            UE_LOG(LogTemp, Display, TEXT("MONSTER_PRECISION %s hits=%d early=%d drift=%.2f recovered=%d"),
                Case.Label, HitEvents, EarlyHits, Drift, Recovered);
            Cleanup(Manager);
            ++Index;
        }
        return false;
    }
private:
    void Cleanup(UMonsterManager* Manager)
    {
        if (Wall.IsValid()) { Wall->Destroy(); Wall.Reset(); }
        if (Monster.IsValid())
        {
            Monster->GetAbilitySystemComponent()->CancelAllAbilities();
            Manager->DespawnMonster(Monster.Get());
            Monster.Reset();
        }
    }
    FAutomationTestBase* Test;
    TWeakObjectPtr<UWorld> World;
    TWeakObjectPtr<ADiaBaseCharacter> Player;
    TWeakObjectPtr<ADiaMonster> Monster;
    TWeakObjectPtr<AActor> Wall;
    TArray<FPrecisionCase> Cases;
    FVector Center, SpawnLocation, RecoveryLocation;
    int32 Index = 0, HitEvents = 0, EarlyHits = 0;
    float Started = 0, CastTime = 0, LastHealth = 0, Drift = 0, OldHealth = 0, OldMaxHealth = 0;
    bool ActionDone = false, SawLock = false, Recovered = false;
};

class FBeginMonsterCombatCommand : public IAutomationLatentCommand
{
public:
    explicit FBeginMonsterCombatCommand(FAutomationTestBase* InTest, bool bInPrecision = false) : Test(InTest), Started(FPlatformTime::Seconds()), bPrecision(bInPrecision) {}
    virtual bool Update() override
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            UWorld* World = Context.World();
            if (!World || World->WorldType != EWorldType::PIE) continue;
            ADiaBaseCharacter* Player = Cast<ADiaBaseCharacter>(UGameplayStatics::GetPlayerPawn(World, 0));
            if (!Player || !Player->GetAbilitySystemComponent() || World->GetTimeSeconds() < 2.f) continue;
            ADiaRoomBase* Room = nullptr;
            for (TActorIterator<ADiaRoomBase> It(World); It; ++It)
                if (It->GetName().Contains(TEXT("RoomEnd")))
                { Room = *It; break; }
            if (!Room) continue;
            // The boss arena is present on every generated floor. Keep both the
            // player and the farthest (750 cm) spawn inside its 1000 cm width.
            const FVector Center = Room->GetActorLocation() + FVector(375, 0, 90.15f);
            Player->SetActorLocation(Center, false, nullptr, ETeleportType::TeleportPhysics);
            for (TActorIterator<ADiaMonster> It(World); It; ++It)
                if (!It->IsHidden()) It->DeactivateAI();
            if (bPrecision) { ADD_LATENT_AUTOMATION_COMMAND(FMonsterPrecisionCommand(Test, World, Player)); }
            else { ADD_LATENT_AUTOMATION_COMMAND(FMonsterCombatCommand(Test, World, Player)); }
            ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
            return true;
        }
        if (FPlatformTime::Seconds() - Started < 30) return false;
        Test->AddError(TEXT("PIE did not produce a player and an open dungeon room within 30 seconds."));
        ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
        return true;
    }
private:
    FAutomationTestBase* Test;
    double Started;
    bool bPrecision;
};
}

bool FDiaMonsterCombatPIETest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FBeginMonsterCombatCommand(this));
    return true;
}
bool FDiaMonsterPrecisionPIETest::RunTest(const FString& Parameters)
{
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
    ADD_LATENT_AUTOMATION_COMMAND(FBeginMonsterCombatCommand(this, true));
    return true;
}
#endif
