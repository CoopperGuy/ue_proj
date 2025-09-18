// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster/DiaMonster.h"
#include "Components/CapsuleComponent.h"
#include "Monster/Controller/DiaAIController.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GameMode/DungeonGameMode.h"
#include "System/ItemSubsystem.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTDecorator.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "DiaComponent/DiaCombatComponent.h"
#include "DiaComponent/DiaStatComponent.h"

#include "UI/HUDWidget.h"

ADiaMonster::ADiaMonster()
{
	PrimaryActorTick.bCanEverTick = true;

	//Default Monster AIController
	AIControllerClass = ADiaAIController::StaticClass();

	AutoPossessAI = EAutoPossessAI::Disabled;

	Tags.Add(FName(TEXT("Monster")));
}

void ADiaMonster::InitializeFromData(const FMonsterInfo& MonsterInfo)
{
	//// 몬스터 기본 정보 설정
	MonsterID = MonsterInfo.MonsterID;
	//
	//// 스탯 설정
	UDiaStatComponent* StatComponent = FindComponentByClass<UDiaStatComponent>();
	if (StatComponent)
	{
		StatComponent->InitializeFromData(MonsterInfo);
	}
		
	// 메시 설정 전에 먼저 컴포넌트 활성화
	USkeletalMeshComponent* MeshComp = GetMesh();



	if (MeshComp)
	{
		// 메시 컴포넌트 초기 설정
		MeshComp->SetVisibility(true);
		MeshComp->SetHiddenInGame(false);
		MeshComp->SetActive(true);

		if (!MonsterInfo.MonsterMesh.IsNull())
		{
			UE_LOG(LogTemp, Log, TEXT("몬스터 [%s] 메시 로드 시도: %s"), *GetName(), *MonsterInfo.MonsterMesh.ToString());
			USkeletalMesh* MonsterMeshAsset = MonsterInfo.MonsterMesh.LoadSynchronous();
			if (MonsterMeshAsset)
			{
				// 메시 설정
				MeshComp->SetSkeletalMesh(MonsterMeshAsset);

				// 중요: 메시 설정 후 강제 업데이트
				MeshComp->RecreateRenderState_Concurrent();

				UE_LOG(LogTemp, Log, TEXT("몬스터 [%s] 메시 설정 완료: %s"), *GetName(), *MonsterMeshAsset->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("몬스터 [%s] 메시 로드 실패: %s"), *GetName(), *MonsterInfo.MonsterMesh.ToString());
			}

			if (MonsterInfo.AnimationInstance)
			{
				MeshComp->SetAnimInstanceClass(MonsterInfo.AnimationInstance);
				UE_LOG(LogTemp, Verbose, TEXT("몬스터 [%s] 애니메이션 클래스 설정 완료"), *GetName());
			}
		}
	}
		
	// AI 컨트롤러 설정
	ADiaAIController* AIController = Cast<ADiaAIController>(GetController());
	if (AIController)
	{
		// Blackboard 설정
		if (!MonsterInfo.BlackboardAsset.IsNull())
		{
			UBlackboardData* BlackboardData = MonsterInfo.BlackboardAsset.LoadSynchronous();
			if (BlackboardData)
			{
				AIController->InitBlackBoardData(this, BlackboardData);
				UE_LOG(LogTemp, Verbose, TEXT("몬스터 [%s] 블랙보드 설정 완료"), *GetName());
			}
		}
		
		// Behavior Tree 설정
		if (!MonsterInfo.BehaviorTree.IsNull())
		{
			UBehaviorTree* BehaviorTreeAsset = MonsterInfo.BehaviorTree.LoadSynchronous();
			if (BehaviorTreeAsset)
			{
				AIController->InitBehaviorTree(BehaviorTreeAsset);
				UE_LOG(LogTemp, Verbose, TEXT("몬스터 [%s] 비헤이비어 트리 실행 시작"), *GetName());
			}
		}
		
		// 컨트롤러 초기화 함수 호출
		AIController->InitializeAI();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("몬스터 [%s]에 유효한 AI 컨트롤러가 없습니다"), *GetName());
	}
	
	// 기본 AI 활성화
	ActivateAI();

	RegisterAllComponents();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);

	SetActorEnableCollision(true);

	GetCharacterMovement()->Velocity = FVector::ZeroVector;

	//스킬 초기화
	InitialSkills = MonsterInfo.MonsterSkills;
	AbilityTags = MonsterInfo.MonsterTags;

	GrantInitialGASAbilities();
}

// AI 비활성화 함수 구현
void ADiaMonster::DeactivateAI()
{
	// AI 컨트롤러 획득
	ADiaAIController* AIController = Cast<ADiaAIController>(GetController());
	if (AIController)
	{
		// AI 로직 중지
		AIController->StopMovement();
		AIController->SetActorTickEnabled(false);
		
		// 비헤이비어 트리 중지
		UBehaviorTreeComponent* BehaviorTreeComp = AIController->FindComponentByClass<UBehaviorTreeComponent>();
		if (IsValid(BehaviorTreeComp))
		{
			BehaviorTreeComp->StopTree();
		}
		
		UE_LOG(LogTemp, Verbose, TEXT("몬스터 [%s]의 AI 비활성화됨"), *GetName());
	}
}

// AI 활성화 함수 구현
void ADiaMonster::ActivateAI()
{
	// AI 컨트롤러 획득
	ADiaAIController* AIController = Cast<ADiaAIController>(GetController());
	if (IsValid(AIController))
	{
		// AI 로직 활성화
		AIController->SetActorTickEnabled(true);
		
		// AI 컨트롤러 초기화
		AIController->InitializeAI();
		
		UE_LOG(LogTemp, Verbose, TEXT("몬스터 [%s]의 AI 활성화됨"), *GetName());
	}
}

// 몬스터 리셋 함수 구현
void ADiaMonster::ResetMonster()
{
	// 체력 및 상태 리셋
	//Health = MaxHealth;
	//bIsDead = false;
	
	// 모든 피해 및 버프 효과 제거
	GetWorldTimerManager().ClearAllTimersForObject(this);
	
	// 애니메이션 초기화
	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Stop(0.25f);
	}
	
	// 자식 컴포넌트 리셋
	TArray<UActorComponent*> Components;
	GetComponents(Components);
	
	for (UActorComponent* Component : Components)
	{
		Component->Activate();
	}
	
	// 캡슐 콜리전 활성화
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	UE_LOG(LogTemp, Verbose, TEXT("몬스터 [%s] 상태 리셋 완료"), *GetName());
}

void ADiaMonster::BeginPlay()
{
	Super::BeginPlay();
	
}

void ADiaMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADiaMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


float ADiaMonster::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	//// CombatStatsComponent를 통해 데미지 처리
	//// 현재 체력 확인 (필요한 경우)
	//float CurrentHealth = 0.0f;
	//float MaxHealth = 0.0f;

	//// 피격 이펙트 재생
	//if (hitEffect)
	//{
	//	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
	//		GetWorld(),
	//		hitEffect,
	//		GetActorLocation(),
	//		GetActorRotation()
	//	);
	//}
	//
	//// 피격 사운드 재생
	//if (hitSound)
	//{
	//	UGameplayStatics::PlaySoundAtLocation(this, hitSound, GetActorLocation());
	//}
	//
	//// 히트 리액션 몽타주 재생
	//if (hitReactionMontage && CurrentHealth > 0.0f)
	//{
	//	PlayCharacterMontage(hitReactionMontage);
	//}
	//
	//// 사망 처리
	//if (CurrentHealth <= 0.0f)
	//{
	//	PlayDieAnimation();
	//}
	
	return ActualDamage;
}

void ADiaMonster::DropItem()
{
	// 몬스터가 죽었을 때 아이템 드랍 로직 구현
	// 예시: 아이템 생성 및 월드에 스폰
	ADungeonGameMode* DungeonGameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
	if (DungeonGameMode)
	{
		// 아이템 데이터 생성
		UItemSubsystem* ItemSubsystem = GetGameInstance()->GetSubsystem<UItemSubsystem>();
		const FItemBase& ItemBaseData = ItemSubsystem->GetItemData("Weapon_Sword_01");
		DungeonGameMode->SpawnItemAtLocation(this, ItemBaseData);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("몬스터 [%s] 아이템 드랍 실패: 던전 게임 모드가 유효하지 않음"), *GetName());
	}
}

void ADiaMonster::SetMonsterCollisionSetup(const FMonsterInfo& MonsterInfo)
{
	//몬스터가 바라보는 방향 및, collision크기 조절 등등
}

void ADiaMonster::UpdateHPGauge(float CurHealth, float MaxHelath)
{
    ADungeonGameMode* DungeonGameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
    
    // 로그 추가 (디버깅용)
    if (!DungeonGameMode)
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHPGauge: DungeonGameMode is null"));
        return;
    }
    
    UHUDWidget* HUD = DungeonGameMode->GetHUDWidget();
    if (HUD)
    {
        float HPPersentage = CurHealth / MaxHelath;
        HUD->UpdateMonsterPercentage(BarType::BT_HP, HPPersentage);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UpdateHPGauge: HUD Widget is null"));
    }
}

void ADiaMonster::PlayDieAnimation()
{
	Super::PlayDieAnimation();
}

void ADiaMonster::Die()
{
	Super::Die();

	//드랍되는아이템을 가져오는 방법 고안 필요하다.
	DropItem();

	ADungeonGameMode* DungeonGameMode = Cast<ADungeonGameMode>(GetWorld()->GetAuthGameMode());
	if (IsValid(DungeonGameMode))
	{
		UHUDWidget* HUD = DungeonGameMode->GetHUDWidget();
		if (IsValid(HUD))
		{
			HUD->SetMonsterHPVisibility(ESlateVisibility::Collapsed);
		}
	}
}

// SetGravity 메서드 추가
void ADiaMonster::SetGravity(bool bEnableGravity)
{
	Super::SetGravity(bEnableGravity);
}