// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/Controller/DiaAIController.h"
#include "GameFramework/PlayerController.h"

#include "EngineUtils.h"

#include "Monster/Controller/AI/BlackboardKeys.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTDecorator.h"
#include "Monster/DiaMonster.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

#include "DiaComponent/DiaOptionManagerComponent.h"

#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

ADiaAIController::ADiaAIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 1500.0f;
		SightConfig->LoseSightRadius = 1800.0f;
		SightConfig->PeripheralVisionAngleDegrees = 360.0f;
		SightConfig->SetMaxAge(5.0f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 900.0f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

		AIPerceptionComp->ConfigureSense(*SightConfig);
		AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ADiaAIController::OnTargetPerceptionUpdated);
	}
}

void ADiaAIController::BeginPlay()
{
	Super::BeginPlay();
	InitBehaviorTree(behaviorTree);
}

void ADiaAIController::InitBehaviorTree(UBehaviorTree* _behaiviortree)
{
	if (_behaiviortree)
	{
		behaviorTree = _behaiviortree;
		RunBehaviorTree(_behaiviortree);
	}
}

void ADiaAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	InitBlackBoardData(InPawn, blackboardData);
}

//우선 타겟 설정으로만 한다.
void ADiaAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Actor && Actor->ActorHasTag(FName(TEXT("Player"))))
	{
		bIsLineOfSight = Stimulus.WasSuccessfullySensed();
		if (bIsLineOfSight)
		{
			SetTarget(Actor);
			LastSeenLocation = Actor->GetActorLocation();
		}
		else
		{
			SetTarget(nullptr);
		}

		if(GetBlackboardComponent())
			GetBlackboardComponent()->SetValueAsBool(BlackboardKeys::Monster::LOS, bIsLineOfSight);
	}
}

void ADiaAIController::InitBlackBoardData(APawn* InPawn, UBlackboardData* _blackboardData)
{
	if (_blackboardData)
	{
		blackboardData = _blackboardData;
		auto blackBorad = Blackboard.Get();
		if (UseBlackboard(_blackboardData, blackBorad))
		{
			Blackboard->SetValueAsVector("HomeLocation", InPawn->GetActorLocation());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BlackboardData or BehaviorTree is not valid!"));
		}
	}
}

void ADiaAIController::InitializeAI()
{
	SetActorTickEnabled(true);
	UBehaviorTreeComponent* BehaviorTreeComp = FindComponentByClass<UBehaviorTreeComponent>();

	if (IsValid(BehaviorTreeComp) && IsValid(behaviorTree))
	{
		BehaviorTreeComp->StartTree(*behaviorTree, EBTExecutionMode::Looped);
	}

	UE_LOG(LogTemp, Verbose, TEXT("몬스터 [%s]의 AI 활성화"), *GetName());
}

void ADiaAIController::UpdateCombatState()
{
	// 소유한 몬스터 가져오기
	const ADiaMonster* ControlledMonster = GetControlledMonster();
	if (!IsValid(ControlledMonster))
	{
		return;
	}
	
	if (!IsValid(CurrentTarget))
	{
		DetectTargets();
	}
}

void ADiaAIController::UpdateTarget()
{
	// 소유한 몬스터 가져오기
	const ADiaMonster* ControlledMonster = GetControlledMonster();
	if (!IsValid(ControlledMonster))
	{
		return;
	}
	
}

void ADiaAIController::ExecuteAttack()
{
	// 타겟 확인
	if (!IsValid(CurrentTarget))
	{
		return;
	}
}

void ADiaAIController::SetTarget(AActor* NewTarget)
{
	if (CurrentTarget == NewTarget)
	{
		return;
	}
	
	CurrentTarget = NewTarget;

	//미사용
	//// 전투 컴포넌트에 타겟 설정
	//ADiaMonster* ControlledMonster = GetControlledMonster();
	//if (!IsValid(ControlledMonster))
	//{
	//	return;
	//}
	//
	//UDiaCombatComponent* CombatComp = ControlledMonster->FindComponentByClass<UDiaCombatComponent>();
	//if (IsValid(CombatComp))
	//{
	//	CombatComp->SetCurrentTarget(NewTarget);
	//}
	
	if (GetBlackboardComponent())
	{
		if (IsValid(NewTarget))
		{
			GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::Monster::TargetActor, NewTarget);
			GetBlackboardComponent()->SetValueAsVector(BlackboardKeys::Monster::LastKnownLocation, NewTarget->GetActorLocation());	
		}
		else
		{
			GetBlackboardComponent()->SetValueAsObject(BlackboardKeys::Monster::TargetActor, nullptr);
			GetBlackboardComponent()->SetValueAsVector(BlackboardKeys::Monster::LastKnownLocation, FVector::ZeroVector);
		}
	}
}

void ADiaAIController::DetectTargets()
{
	// 가장 가까운 플레이어 찾기
	AActor* NearestPlayer = FindNearestPlayer();
	if (IsValid(NearestPlayer))
	{
		// 감지 거리 내에 있는지 확인
		FVector MyLocation = GetPawn()->GetActorLocation();
		FVector PlayerLocation = NearestPlayer->GetActorLocation();
		float Distance = FVector::Dist(MyLocation, PlayerLocation);
		
		if (Distance <= DetectionRadius)
		{
			SetTarget(NearestPlayer);
		}
	}
}

AActor* ADiaAIController::FindNearestPlayer()
{
	AActor* NearestPlayer = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();
	
	// 월드에서 모든 플레이어 컨트롤러 찾기
	//TActorIterator < 특정 타입의 actor를 찾는 것
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		APlayerController* PC = *It;
		if (!IsValid(PC))
		{
			continue;
		}
		
		APawn* PlayerPawn = PC->GetPawn();
		if (!IsValid(PlayerPawn))
		{
			continue;
		}
		
		// 거리 계산
		FVector MyLocation = GetPawn()->GetActorLocation();
		FVector PlayerLocation = PlayerPawn->GetActorLocation();
		float Distance = FVector::Dist(MyLocation, PlayerLocation);
		
		// 더 가까운 플레이어 업데이트
		if (Distance < NearestDistance)
		{
			NearestPlayer = PlayerPawn;
			NearestDistance = Distance;
		}
	}
	
	return NearestPlayer;
}

float ADiaAIController::GetDistanceToTarget() const
{
	if (!IsValid(CurrentTarget) || !IsValid(GetPawn()))
	{
		return TNumericLimits<float>::Max();
	}

	FVector MyLocation = GetPawn()->GetActorLocation();
	FVector TargetLocation = CurrentTarget->GetActorLocation();

	return FVector::Dist(MyLocation, TargetLocation);
}

ADiaMonster* ADiaAIController::GetControlledMonster() const
{
	return Cast<ADiaMonster>(GetPawn());
}
