// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MonsterPoolSubsystem.h"
#include "Monster/DiaMonster.h"
#include "System/MonsterManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Types/DiaMonsterTable.h"

// 로그 매크로 정의 - 코드 일관성 유지
#define MONSTER_POOL_LOG(Verbosity, Format, ...) UE_LOG(LogTemp, Verbosity, TEXT("MonsterPool: " Format), ##__VA_ARGS__)

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

			MONSTER_POOL_LOG(Log, TEXT("Initialized pools for %d monster types"), MonsterCacheMap.Num());
		}
		else
		{
			MONSTER_POOL_LOG(Warning, TEXT("Failed to get MonsterManager"));
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
	// 유효한 몬스터 ID 체크
	if (MonsterID.IsNone())
	{
		MONSTER_POOL_LOG(Error, TEXT("AcquireMonster: Invalid monster ID"));
		return nullptr;
	}

	// 풀이 없으면 생성
	if (!MonsterPools.Contains(MonsterID))
	{
		PreparePool(MonsterID, DefaultInitialPoolSize);
	}

	FMonsterPoolController* Pool = MonsterPools.Find(MonsterID);
	if (!Pool)
	{
		MONSTER_POOL_LOG(Error, TEXT("AcquireMonster: Failed to find or create pool for %s"), *MonsterID.ToString());
		return nullptr;
	}

	// 사용 가능한 몬스터 찾기
	ADiaMonster* AvailableMonster = nullptr;
	FPooledMonster* AvailablePoolMonster = nullptr;

	for (auto& PoolMonster : Pool->MonsterPool)
	{
		if (!PoolMonster.bInUse && IsValid(PoolMonster.Monster))
		{
			AvailableMonster = PoolMonster.Monster;
			AvailablePoolMonster = &PoolMonster;
			break;
		}
	}

	// 사용 가능한 몬스터가 없으면 풀 확장
	if (!AvailableMonster)
	{
		int32 NewCount = FMath::CeilToInt(Pool->MonsterPool.Num() * PoolExpansionFactor) - Pool->MonsterPool.Num();
		NewCount = FMath::Max(1, NewCount); // 최소 1개 이상 추가
		ExpandPool(MonsterID, NewCount);

		// 확장 후 다시 검색
		for (auto& PoolMonster : Pool->MonsterPool)
		{
			if (!PoolMonster.bInUse && IsValid(PoolMonster.Monster))
			{
				AvailableMonster = PoolMonster.Monster;
				AvailablePoolMonster = &PoolMonster;
				break;
			}
		}
	}

	// 몬스터를 찾았으면 초기화 및 반환
	if (AvailableMonster && AvailablePoolMonster)
	{
		AvailablePoolMonster->bInUse = true;
		AvailablePoolMonster->LastUsedTime = FDateTime::Now();
		Pool->LastPoolUsedTime = FDateTime::Now();
		Pool->ActiveCount++;

		InitializeMonster(AvailableMonster, Location, MonsterID);
		MONSTER_POOL_LOG(Verbose, TEXT("Acquired monster %s from pool"), *AvailableMonster->GetName());
		return AvailableMonster;
	}

	MONSTER_POOL_LOG(Warning, TEXT("Failed to acquire monster for ID %s"), *MonsterID.ToString());
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
		 MONSTER_POOL_LOG(Warning, TEXT("몬스터 [%s] 텔레포트 실패. 위치: %s"), *Monster->GetName(), *FinalSpawnLocation.ToString());
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
	if (Count <= 0 || MonsterID.IsNone())
	{
		return;
	}

	// 최대 크기 제한 체크
	if (Count > MaxPoolSizePerType)
	{
		MONSTER_POOL_LOG(Warning, TEXT("ExpandPool: Requested count %d exceeds MaxPoolSizePerType %d"), 
			Count, MaxPoolSizePerType);
		Count = MaxPoolSizePerType;
	}

	if (!MonsterManager.IsValid())
	{
		MONSTER_POOL_LOG(Error, TEXT("ExpandPool: MonsterManager not valid"));
		return;
	}

	FMonsterPoolController* Pool = MonsterPools.Find(MonsterID);
	if (!Pool)
	{
		MONSTER_POOL_LOG(Error, TEXT("ExpandPool: Pool not found for monster ID %s"), *MonsterID.ToString());
		return;
	}


	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;

	UMonsterManager* MM = GI->GetSubsystem<UMonsterManager>();
	if (!MM)
	{
		MONSTER_POOL_LOG(Error, TEXT("InitializeMonster: MonsterManager not found in GameInstance"));
		return;
	}

	const FMonsterInfo* MonsterInfo = MM->GetMonsterInfo(MonsterID);
	if (!MonsterInfo)
	{
		MONSTER_POOL_LOG(Warning, TEXT("InitializeMonster: MonsterInfo not found for ID %s"), *MonsterID.ToString());
		return;
	}

	int32 CreatedCount = 0;
	for (int32 i = 0; i < Count; i++)
	{
		ADiaMonster* NewMonster = CreateNewMonster(MonsterID);
		if (IsValid(NewMonster))
		{
			NewMonster->InitializeFromData(*MonsterInfo);
			ResetMonsterState(NewMonster);

			FPooledMonster PooledMonster;
			PooledMonster.Monster = NewMonster;
			PooledMonster.bInUse = false;
			PooledMonster.LastUsedTime = FDateTime::Now();
			Pool->MonsterPool.Emplace(PooledMonster);
			CreatedCount++;
		}
	}

	MONSTER_POOL_LOG(Log, TEXT("ExpandPool: Created %d/%d new monsters for pool %s"), 
		CreatedCount, Count, *MonsterID.ToString());
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
			MONSTER_POOL_LOG(Error, TEXT("CreateNewMonster: Failed to get MonsterManager"));
			return nullptr;
		}
	}

	// 몬스터 정보 확인
	const TMap<FName, FMonsterInfo>& MonsterCache = MonsterManager->GetMonsterCache();
	if (!MonsterCache.Contains(MonsterID))
	{
		MONSTER_POOL_LOG(Error, TEXT("CreateNewMonster: Monster ID %s not found in cache"),
			*MonsterID.ToString());
		return nullptr;
	}

	// 새 몬스터 생성
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADiaMonster* NewMonster = GetWorld()->SpawnActor<ADiaMonster>(
		ADiaMonster::StaticClass(),
		FVector(0, 0, -10000), // 멀리 스폰
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (IsValid(NewMonster))
	{
		MONSTER_POOL_LOG(Verbose, TEXT("CreateNewMonster: Created new monster %s"), *NewMonster->GetName());
	}

	return NewMonster;
}

void UMonsterPoolSubsystem::InitializeMonster(ADiaMonster* Monster, const FVector& Location, FName MonsterID)
{
	if (!IsValid(Monster))
	{
		MONSTER_POOL_LOG(Error, TEXT("InitializeMonster: Invalid Monster instance"));
		return;
	}


	TArray<UActorComponent*> Components;
	Monster->GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		Component->Activate();
	}

	// 컴포넌트 가져오기 (유효성 검사)
	UCapsuleComponent* Capsule = Monster->GetCapsuleComponent();
	UCharacterMovementComponent* MoveComp = Monster->GetCharacterMovement();
	if (!Capsule || !MoveComp)
	{
		MONSTER_POOL_LOG(Error, TEXT("InitializeMonster: Missing Capsule or MovementComponent on %s"), *Monster->GetName());
		return;
	}

	// 1. 상태 초기화 (가장 먼저 수행)
	Monster->ResetMonster();

	// MonsterManager에서 정보 가져오기
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (!GI) return;     

	UMonsterManager* MM = GI->GetSubsystem<UMonsterManager>();
	if (!MM)
	{
		MONSTER_POOL_LOG(Error, TEXT("InitializeMonster: MonsterManager not found in GameInstance"));
		return;
	}

	const FMonsterInfo* MonsterInfo = MM->GetMonsterInfo(MonsterID);
	if (!MonsterInfo)
	{
		MONSTER_POOL_LOG(Warning, TEXT("InitializeMonster: MonsterInfo not found for ID %s"), *MonsterID.ToString());
		return;
	}

	// 2. AI 컨트롤러 스폰 및 빙의 (위치 설정 전)
	if (!Monster->GetController())
	{
		Monster->SpawnDefaultController();
	}

	// 3. 데이터 초기화
	Monster->InitializeFromData(*MonsterInfo);

	// 4. 위치 설정 (텔레포트 사용)
	FVector FinalSpawnLocation = Location + FVector(0, 0, Capsule->GetScaledCapsuleHalfHeight() * 0.1f);
	bool bTeleported = Monster->SetActorLocation(FinalSpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
	if (!bTeleported)
	{
		MONSTER_POOL_LOG(Warning, TEXT("InitializeMonster: ResetPhysics failed for %s, trying ResetPhysics"), *Monster->GetName());
		Monster->SetActorLocation(FinalSpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
	}
	
	// 6. 이동 컴포넌트 활성화 및 중력 설정
	MoveComp->SetMovementMode(MOVE_Walking);
	MoveComp->Activate(true);

	// 7. AI 로직 활성화
	Monster->ActivateAI();

	// 8. 액터 활성화 및 표시 설정
	Monster->SetActorHiddenInGame(false);
	Monster->SetActorTickEnabled(true);
	Monster->SetActorEnableCollision(true);
	Monster->SetGravity(true);
	Monster->SetLifeSpan(0.0f);

}

void UMonsterPoolSubsystem::ReturnMonsterToPool(ADiaMonster* Monster)
{
	if (!IsValid(Monster))
	{
		MONSTER_POOL_LOG(Warning, TEXT("ReturnMonsterToPool: Trying to release invalid monster"));
		return;
	}

	bool bFound = false;

	// 모든 풀 검색
	for (auto& Pair : MonsterPools)
	{
		FMonsterPoolController& Pool = Pair.Value;

		for (auto& PooledMonster : Pool.MonsterPool)
		{
			if (PooledMonster.Monster && PooledMonster.Monster->GetUniqueID() == Monster->GetUniqueID())
			{
				// 이미 비활성화된 몬스터인지 확인
				if (!PooledMonster.bInUse)
				{
					MONSTER_POOL_LOG(Warning, TEXT("ReturnMonsterToPool: Monster %s already released"),
						*Monster->GetName());
					return;
				}

				// 몬스터 재사용을 위한 리셋
				ResetMonsterState(Monster);

				// 사용 해제 상태로 전환
				PooledMonster.bInUse = false;
				PooledMonster.LastUsedTime = FDateTime::MinValue();
				Pool.ActiveCount--;
				Pool.LastPoolUsedTime = FDateTime::Now();
				
				bFound = true;
				MONSTER_POOL_LOG(Verbose, TEXT("ReturnMonsterToPool: Released monster %s to pool"), *Monster->GetName());
				break;
			}
		}

		if (bFound) break;
	}

	if (!bFound)
	{
		MONSTER_POOL_LOG(Warning, TEXT("ReturnMonsterToPool: Monster %s not found in any pool"), *Monster->GetName());
	}
}

//풀 정리
void UMonsterPoolSubsystem::CleanupUnusedMonsters()
{
	const FDateTime CurrentTime = FDateTime::Now();
	TArray<FName> EmptyPoolsToRemove;

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
			MONSTER_POOL_LOG(Log, TEXT("CleanupUnusedMonsters: Removing unused pool for %s"),
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
				MONSTER_POOL_LOG(Log, TEXT("CleanupUnusedMonsters: Cleaning up %d unused monsters from %s pool"),
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

void UMonsterPoolSubsystem::DebugMonsterPool(FName MonsterID)
{
	if (!MonsterPools.Contains(MonsterID))
	{
		MONSTER_POOL_LOG(Warning, TEXT("DebugMonsterPool: Pool for %s not found"), *MonsterID.ToString());
		return;
	}

	FMonsterPoolController* Pool = MonsterPools.Find(MonsterID);
	MONSTER_POOL_LOG(Log, TEXT("===== Pool Debug for %s ====="), *MonsterID.ToString());
	MONSTER_POOL_LOG(Log, TEXT("Total monsters: %d, Active: %d"), Pool->MonsterPool.Num(), Pool->ActiveCount);

	int32 Active = 0;
	int32 Invalid = 0;
	int32 WithController = 0;

	for (int32 i = 0; i < Pool->MonsterPool.Num(); ++i)
	{
		auto& PoolMonster = Pool->MonsterPool[i];
		if (PoolMonster.bInUse) Active++;
		
		if (!IsValid(PoolMonster.Monster))
		{
			Invalid++;
			continue;
		}
		
		if (PoolMonster.Monster->GetController()) WithController++;

		FString Status = PoolMonster.bInUse ? TEXT("사용중") : TEXT("대기중");
		MONSTER_POOL_LOG(Log, TEXT("[%d] %s - %s, AI: %s"), 
			i, 
			*PoolMonster.Monster->GetName(), 
			*Status,
			PoolMonster.Monster->GetController() ? TEXT("있음") : TEXT("없음"));
	}

	MONSTER_POOL_LOG(Log, TEXT("Active: %d, Invalid: %d, With Controller: %d"), Active, Invalid, WithController);
	MONSTER_POOL_LOG(Log, TEXT("================================="));
}
