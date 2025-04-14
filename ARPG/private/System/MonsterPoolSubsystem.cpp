// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MonsterPoolSubsystem.h"
#include "Monster/DiaMonster.h"
#include "System/MonsterManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/DiaMonsterTable.h"


void UMonsterPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (IsValid(GI))
	{
		MonsterManager = GI->GetSubsystem<UMonsterManager>();

		// 몬스터 타입별로 기본 풀 준비
		if (MonsterManager.IsValid())
		{
			const TMap<FName, FMonsterInfo>& MonsterCacheMap = MonsterManager->GetMonsterCache();
			for (const auto& Pair : MonsterCacheMap)
			{
				PreparePool(Pair.Key, DefaultInitialPoolSize);
			}

			UE_LOG(LogTemp, Log, TEXT("MonsterPoolSubsystem: Initialized pools for %d monster types"), MonsterCacheMap.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MonsterPoolSubsystem: Failed to get MonsterManager"));
		}
	}

	// 미사용 몬스터 정리를 위한 타이머 설정
	GetWorld()->GetTimerManager().SetTimer(
		CleanupTimerHandle,
		this,
		&UMonsterPoolSubsystem::CleanupUnusedMonsters,
		CleanupInterval,
		true
	);
}

void UMonsterPoolSubsystem::Deinitialize()
{
	// 타이머 정리
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(CleanupTimerHandle);
	}

	// 모든 몬스터 풀 정리
	for (auto& Pair : MonsterPools)
	{
		for (auto& PooledMonster : Pair.Value.MonsterPool)
		{
			if (IsValid(PooledMonster.Monster))
			{
				PooledMonster.Monster->Destroy();
				PooledMonster.Monster = nullptr;
			}
		}

		Pair.Value.MonsterPool.Empty();
		Pair.Value.ActiveCount = 0;
	}

	MonsterPools.Empty();
	MonsterManager.Reset();

	Super::Deinitialize();
}

//몬스터 풀에서 몬스터를 받고 반환
ADiaMonster* UMonsterPoolSubsystem::AcquireMonster(FName MonsterID, const FVector& Location)
{
	if (!MonsterPools.Contains(MonsterID))
	{
		PreparePool(MonsterID, 5);
	}

	FMonsterPoolController* Pool = MonsterPools.Find(MonsterID);
	if (!Pool)
	{
		return nullptr;
	}

	for (auto& PoolMonster : Pool->MonsterPool)
	{
		if (!PoolMonster.bInUse && IsValid(PoolMonster.Monster))
		{
			PoolMonster.bInUse = true;
			PoolMonster.LastUsedTime = FDateTime::Now();
			Pool->LastPoolUsedTime = FDateTime::Now();

			Pool->ActiveCount++;

			InitializeMonster(PoolMonster.Monster, Location, MonsterID);

			return PoolMonster.Monster;
		}
	}


	int32 NewCount = FMath::CeilToInt(Pool->MonsterPool.Num() * PoolExpansionFactor) - Pool->MonsterPool.Num();
	NewCount = FMath::Max(1, NewCount); // 최소 1개 이상 추가
	//if (Pool->MonsterPool.Num() >= MaxPoolSizePerType)
	//{
	//	NewCount = 0;
	//}
	ExpandPool(MonsterID, NewCount);

	for (auto& PoolMonster : Pool->MonsterPool)
	{
		if (!PoolMonster.bInUse && IsValid(PoolMonster.Monster))
		{
			PoolMonster.bInUse = true;
			PoolMonster.LastUsedTime = FDateTime::Now();
			Pool->LastPoolUsedTime = FDateTime::Now();

			Pool->ActiveCount++;

			InitializeMonster(PoolMonster.Monster, Location, MonsterID);

			return PoolMonster.Monster;
		}
	}

	return nullptr;
}

//생성된 몬스터를 풀로 반환
void UMonsterPoolSubsystem::ResetMonsterState(ADiaMonster* Monster)
{
	if (!IsValid(Monster))
	{
		return;
	}

	// 상태 초기화
	Monster->SetActorHiddenInGame(true);
	Monster->SetActorEnableCollision(false);
	Monster->SetActorTickEnabled(false);
	Monster->SetGravity(false);


	// 멀리 이동
	FVector FinalSpawnLocation = InactiveLocation;
	bool bTeleported = Monster->SetActorLocation(FinalSpawnLocation, false, nullptr, ETeleportType::TeleportPhysics);
	if (!bTeleported)
	{
		 UE_LOG(LogTemp, Warning, TEXT("몬스터 [%s] 텔레포트 실패. 위치: %s"), *Monster->GetName(), *FinalSpawnLocation.ToString());
		 // 텔레포트 실패 시 강제로 위치 설정 시도
		 Monster->SetActorLocation(FinalSpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
	}

	// AI 비활성화
	Monster->DeactivateAI();

	// 체력 등 기본 속성 초기화
	Monster->ResetMonster();

	// 수명 제한 제거
	Monster->SetLifeSpan(0.0f); 

	// 컨트롤러가 있다면 제거 (혹시 생성된 경우를 대비)
	if (Monster->GetController())
	{
		Monster->DetachFromControllerPendingDestroy();
	}
}


//풀 생성
void UMonsterPoolSubsystem::PreparePool(FName MonsterID, int32 Count)
{
	if (MonsterID.IsNone() || Count <= 0)
	{
		return;
	}

	//풀이 없으면 새로 추가한다.
	if (!MonsterPools.Contains(MonsterID))
	{
		FMonsterPoolController NewPool;
		MonsterPools.Add(MonsterID, NewPool);
	}

	FMonsterPoolController* Pool = MonsterPools.Find(MonsterID);

	if (!Pool)
	{
		return;
	}

	// 이미 충분한 몬스터가 있는지 확인
	int32 CurrentCount = Pool->MonsterPool.Num();
	if (CurrentCount >= Count)
	{
		return;
	}

	// 추가 필요한 몬스터 수
	int32 ToAdd = FMath::Min(Count - CurrentCount, MaxPoolSizePerType - CurrentCount);
	if (ToAdd > 0)
	{
		ExpandPool(MonsterID, ToAdd);
	}
}

//풀 확장
void UMonsterPoolSubsystem::ExpandPool(FName MonsterID, int32 Count)
{

	if (Count > MaxPoolSizePerType)
	{
		UE_LOG(LogTemp, Error, TEXT("MonsterPoolSubsystem: MonsterManager ExpandPool Error!!!!!!!!!"));
	}

	if (MonsterID.IsNone() || Count <= 0)
	{
		return;
	}
	if (!MonsterManager.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("MonsterPoolSubsystem: MonsterManager not valid"));
		return;
	}
	FMonsterPoolController* Pool = MonsterPools.Find(MonsterID);
	if (!Pool)
	{
		UE_LOG(LogTemp, Error, TEXT("MonsterPoolSubsystem: Pool not found for monster ID %s"), *MonsterID.ToString());
		return;
	}
	for (int32 i = 0; i < Count; i++)
	{
		ADiaMonster* NewMonster = CreateNewMonster(MonsterID);
		if (IsValid(NewMonster))
		{
			ResetMonsterState(NewMonster);

			FPooledMonster PooledMonster;
			PooledMonster.Monster = NewMonster;
			PooledMonster.bInUse = false;
			PooledMonster.LastUsedTime = FDateTime::Now();
			Pool->MonsterPool.Emplace(PooledMonster);
			//생성된 Monster의 Name을 출력하는 로그
			UE_LOG(LogTemp, Log, TEXT("MonsterPoolSubsystem: Created new monster %s"), *NewMonster->GetName());
		}
	}
}

//새로운 몬스터 생성
ADiaMonster* UMonsterPoolSubsystem::CreateNewMonster(FName MonsterID)
{
	// 매니저 유효성 확인
	if (!MonsterManager.IsValid())
	{
		if (UGameInstance* GI = GetWorld()->GetGameInstance())
		{
			MonsterManager = GI->GetSubsystem<UMonsterManager>();
		}

		if (!MonsterManager.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("MonsterPoolSubsystem: Failed to get MonsterManager"));
			return nullptr;
		}
	}

	// 몬스터 정보 확인
	const TMap<FName, FMonsterInfo>& MonsterCache = MonsterManager->GetMonsterCache();
	if (!MonsterCache.Contains(MonsterID))
	{
		UE_LOG(LogTemp, Error, TEXT("MonsterPoolSubsystem: Monster ID %s not found in cache"),
			*MonsterID.ToString());
		return nullptr;
	}

	const FMonsterInfo& MonsterInfo = MonsterCache[MonsterID];

	// 새 몬스터 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADiaMonster* NewMonster = GetWorld()->SpawnActor<ADiaMonster>(
		ADiaMonster::StaticClass(),
		FVector(0, 0, -10000), // 멀리 스폰
		FRotator::ZeroRotator,
		SpawnParams
	);

	return NewMonster;
}

void UMonsterPoolSubsystem::InitializeMonster(ADiaMonster* Monster, const FVector& Location, FName MonsterID)
{
	if (!IsValid(Monster))
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeMonster: Invalid Monster instance"));
		return;
	}
	UE_LOG(LogTemp, Log, TEXT("InitializeMonster: Initializing monster %s at %s"), *Monster->GetName(), *Location.ToString());
	// 0. 컴포넌트 가져오기 (유효성 검사)
	UCapsuleComponent* Capsule = Monster->GetCapsuleComponent();
	UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement();
	if (!Capsule || !MoveComp)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeMonster: Missing Capsule or MovementComponent on %s"), *Monster->GetName());
		return;
	}

	// 1. 상태 초기화 (가장 먼저 수행)
	Monster->ResetMonster(); // 몬스터 리셋 함수 호출 (체력, 애니메이션 등)

	
	//GameinstanceSubsystem 가져오기
	//해당 서브시스템은 게임 인스턴스에 등록되어 있어야 한다.
	//해당 서브 시세틈은 게임 인스턴스르 통해 가져올 수 있다.
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI)	return;		

	UMonsterManager* MM = GI->GetSubsystem<UMonsterManager>();
	if (!MM)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeMonster: MonsterManager not found in GameInstance"));
		return;
	}

	const FMonsterInfo* MonsterInfo = MM->GetMonsterInfo(MonsterID);

	// 2. AI 컨트롤러 스폰 및 빙의 (위치 설정 전)
	// 중요: AI 컨트롤러가 없으면 스폰합니다. 이미 있다면 스폰하지 않습니다.
	if (!Monster->GetController())
	{
		//UE_LOG(LogTemp, Log, TEXT("InitializeMonster: Spawning new AIController for %s"), *Monster->GetName());
		Monster->SpawnDefaultController(); // 기본 AI 컨트롤러 스폰
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("InitializeMonster: Monster %s already has a controller."), *Monster->GetName());
	}

	// 3. 데이터 초기화 (위치 설정 전)
	if (MonsterInfo)
	{
		Monster->InitializeFromData(*MonsterInfo); // 메시, 애니메이션, AI 데이터 설정
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeMonster: MonsterInfo not found for ID %s"), *MonsterID.ToString());
		// 기본 메시나 데이터 설정 또는 에러 처리
		return; // 데이터 없으면 초기화 중단
	}

	// 4. 위치 설정 (텔레포트 사용)
	FVector FinalSpawnLocation = Location + FVector(0, 0, Capsule->GetScaledCapsuleHalfHeight() * 0.1f); // 캡슐 높이의 10% 정도 위
	//UE_LOG(LogTemp, Log, TEXT("InitializeMonster: Teleporting %s to %s"), *Monster->GetName(), *FinalSpawnLocation.ToString());
	bool bTeleported = Monster->SetActorLocation(FinalSpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
	if (!bTeleported)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeMonster: ResetPhysics failed for %s, trying ResetPhysics"), *Monster->GetName());
		Monster->SetActorLocation(FinalSpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
	}

	// 5. 콜리전 활성화 (위치 설정 후)
	//UE_LOG(LogTemp, Log, TEXT("InitializeMonster: Enabling collision for %s"), *Monster->GetName());
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// Capsule->SetCollisionProfileName(DefaultCollisionProfileName); // 필요시 프로파일 복원
	
	// 6. 이동 컴포넌트 활성화 및 중력 설정 (가장 마지막 단계)
	//UE_LOG(LogTemp, Log, TEXT("InitializeMonster: Activating MovementComponent for %s"), *Monster->GetName());
	MoveComp->SetMovementMode(MOVE_Walking); // 중력 적용 시작
	MoveComp->Activate(true); // 컴포넌트 활성화

	// 7. AI 로직 활성화 (모든 설정 완료 후)
	//UE_LOG(LogTemp, Log, TEXT("InitializeMonster: Activating AI for %s"), *Monster->GetName());
	Monster->ActivateAI(); // AI 활성화 (비헤이비어 트리 시작 등)

	// 8. 액터 활성화 및 표시 설정
	Monster->SetActorHiddenInGame(false);
	Monster->SetActorTickEnabled(true);
	Monster->SetActorEnableCollision(true);
	Monster->SetGravity(true);
	Monster->SetLifeSpan(0.0f); 
	// 수명 제한 제거
	// 사용과 동시에 세팅을 하니 문제가 발생하는 걸수도
	//delegate를 이용 할 지 고민해봐야 할 사안이다.
}

void UMonsterPoolSubsystem::ReturnMonsterToPool(ADiaMonster* Monster)
{
	if (!IsValid(Monster))
	{
		UE_LOG(LogTemp, Warning, TEXT("MonsterPoolSubsystem: Trying to release invalid monster"));
		return;
	}

	// 모든 풀 검색
	for (auto& Pair : MonsterPools)
	{
		FMonsterPoolController& Pool = Pair.Value;

		for (auto& PooledMonster : Pool.MonsterPool)
		{
			if (PooledMonster.Monster == Monster)
			{
				// 이미 비활성화된 몬스터인지 확인
				if (!PooledMonster.bInUse)
				{
					UE_LOG(LogTemp, Warning, TEXT("MonsterPoolSubsystem: Monster %s already released"),
						*Monster->GetName());
					return;
				}

				// 몬스터 재사용을 위한 리셋
				ResetMonsterState(Monster);

				// 사용 해제 상태로 전환
				PooledMonster.bInUse = false;
				PooledMonster.LastUsedTime = FDateTime::MinValue(); // 사용 시간 초기화
				Pool.ActiveCount--;

				Pool.LastPoolUsedTime = FDateTime::Now();

				// --- 중요: 상태 비활성화 ---
				Monster->DeactivateAI(); // AI 비활성화 (BT 중지, 틱 비활성화)
				Monster->GetCharacterMovement()->Deactivate(); // 이동 컴포넌트 비활성화
				Monster->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 끄기
				Monster->SetActorLocation(InactiveLocation); // 비활성 위치로 이동
				Monster->SetActorHiddenInGame(true); // 숨기기
				Monster->SetActorTickEnabled(false); // 틱 끄기

				UE_LOG(LogTemp, Verbose, TEXT("MonsterPoolSubsystem: Released monster %s to pool"),
					*Monster->GetName());
				return;
			}
		}
	}

}
//풀 정리
void UMonsterPoolSubsystem::CleanupUnusedMonsters()
{
	const FDateTime CurrentTime = FDateTime::Now();
	TArray<FName> EmptyPoolsToRemove;

	UE_LOG(LogTemp, Verbose, TEXT("MonsterPoolSubsystem: Running cleanup for unused monsters"));

	// 모든 풀 검사
	for (auto& Pair : MonsterPools)
	{
		FName MonsterID = Pair.Key;
		FMonsterPoolController& Pool = Pair.Value;

		// 이 풀 전체가 오랫동안 사용되지 않았는지 확인
		FTimespan PoolIdleTime = CurrentTime - Pool.LastPoolUsedTime;
		if (PoolIdleTime.GetTotalSeconds() > CleanupInterval * 2 && Pool.ActiveCount == 0)
		{
			// 오랫동안 사용되지 않은 풀은 완전히 제거
			UE_LOG(LogTemp, Log, TEXT("MonsterPoolSubsystem: Removing unused pool for %s"),
				*MonsterID.ToString());

			for (auto& PooledMonster : Pool.MonsterPool)
			{
				if (IsValid(PooledMonster.Monster))
				{
					PooledMonster.Monster->Destroy();
					PooledMonster.Monster = nullptr;
				}
			}

			EmptyPoolsToRemove.Add(MonsterID);
			continue;
		}

		// 개별 미사용 몬스터 정리 (반은 남기고)
		if (Pool.MonsterPool.Num() > DefaultInitialPoolSize && Pool.ActiveCount < Pool.MonsterPool.Num() / 2)
		{
			int32 InactiveCount = Pool.MonsterPool.Num() - Pool.ActiveCount;
			int32 TargetInactiveCount = DefaultInitialPoolSize;
			int32 ToRemove = FMath::Max(0, InactiveCount - TargetInactiveCount);

			if (ToRemove > 0)
			{
				UE_LOG(LogTemp, Log, TEXT("MonsterPoolSubsystem: Cleaning up %d unused monsters from %s pool"),
					ToRemove, *MonsterID.ToString());

				// 가장 오래 사용되지 않은 것부터 제거
				int32 Removed = 0;
				for (int32 i = Pool.MonsterPool.Num() - 1; i >= 0 && Removed < ToRemove; --i)
				{
					auto& PooledMonster = Pool.MonsterPool[i];
					if (!PooledMonster.bInUse)
					{
						if (IsValid(PooledMonster.Monster))
						{
							PooledMonster.Monster->Destroy();
						}
						Pool.MonsterPool.RemoveAt(i);
						Removed++;
					}
				}
			}
		}
	}

	// 빈 풀 제거
	for (const FName& PoolToRemove : EmptyPoolsToRemove)
	{
		MonsterPools.Remove(PoolToRemove);
	}
}
