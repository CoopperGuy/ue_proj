#include "Monster/Controller/AI/BTTask_PlayGAS.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "GAS/DiaGASHelper.h"
#include "Monster/Controller/DiaAIController.h"
#include "Monster/DiaMonster.h"

UBTTask_PlayGAS::UBTTask_PlayGAS()
{
    NodeName = TEXT("Play GameplayAbilitySystemSkill");
    bNotifyTick = true;
    bNotifyTaskFinished = true;
    BlackboardKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PlayGAS, BlackboardKey));
}

EBTNodeResult::Type UBTTask_PlayGAS::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    auto* Memory = CastInstanceNodeMemory<FBTPlayGASTaskMemory>(NodeMemory);
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Memory || !Blackboard) return EBTNodeResult::Failed;

    Memory->CurrentAbilityHandle = FGameplayAbilitySpecHandle();
    Memory->SkillID = Blackboard->GetValueAsInt(BlackboardKey.SelectedKeyName);
    ADiaMonster* Monster = OwnerComp.GetAIOwner() ? Cast<ADiaMonster>(OwnerComp.GetAIOwner()->GetPawn()) : nullptr;
    UAbilitySystemComponent* ASC = Monster ? Monster->GetAbilitySystemComponent() : nullptr;
    UDiaSkillManagerComponent* Skills = Monster ? Monster->GetSkillManagerComponent() : nullptr;
    if (!ASC || !ASC->AbilityActorInfo.IsValid() || !Skills || Memory->SkillID == 0)
        return EBTNodeResult::Failed;

    FGameplayAbilitySpec* Spec = UDiaGASHelper::GetAbilitySpecBySkillID(ASC, Memory->SkillID);
    if (!Spec) return EBTNodeResult::Failed;
    Memory->CurrentAbilityHandle = Spec->Handle;

    // Observe abilities started by another task as well as abilities started here.
    // Polling the spec also handles synchronous completion without retaining a raw
    // pointer to BT node memory in an ASC delegate after the tree has stopped.
    if (!Spec->IsActive() &&
        (!Skills->CanActivateAbilityBySkillID(Memory->SkillID) || !Skills->TryActivateAbilityBySkillID(Memory->SkillID)))
        return EBTNodeResult::Failed;

    Spec = ASC->FindAbilitySpecFromHandle(Memory->CurrentAbilityHandle);
    return Spec && Spec->IsActive() ? EBTNodeResult::InProgress : EBTNodeResult::Succeeded;
}

void UBTTask_PlayGAS::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
    const auto* Memory = CastInstanceNodeMemory<FBTPlayGASTaskMemory>(NodeMemory);
    ADiaMonster* Monster = OwnerComp.GetAIOwner() ? Cast<ADiaMonster>(OwnerComp.GetAIOwner()->GetPawn()) : nullptr;
    UAbilitySystemComponent* ASC = Monster ? Monster->GetAbilitySystemComponent() : nullptr;
    if (!Memory || !ASC)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Memory->CurrentAbilityHandle);
    if (!Spec || !Spec->IsActive())
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

EBTNodeResult::Type UBTTask_PlayGAS::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    const auto* Memory = CastInstanceNodeMemory<FBTPlayGASTaskMemory>(NodeMemory);
    ADiaMonster* Monster = OwnerComp.GetAIOwner() ? Cast<ADiaMonster>(OwnerComp.GetAIOwner()->GetPawn()) : nullptr;
    if (Memory && Monster)
    {
        if (UAbilitySystemComponent* ASC = Monster->GetAbilitySystemComponent())
            ASC->CancelAbilityHandle(Memory->CurrentAbilityHandle);
    }
    return EBTNodeResult::Aborted;
}

void UBTTask_PlayGAS::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    auto* Memory = CastInstanceNodeMemory<FBTPlayGASTaskMemory>(NodeMemory);
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (Memory && Blackboard && Blackboard->GetValueAsInt(BlackboardKey.SelectedKeyName) == Memory->SkillID)
        Blackboard->SetValueAsInt(BlackboardKey.SelectedKeyName, 0);
    if (Memory)
    {
        Memory->CurrentAbilityHandle = FGameplayAbilitySpecHandle();
        Memory->SkillID = 0;
    }
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
