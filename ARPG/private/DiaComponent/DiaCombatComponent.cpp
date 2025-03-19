// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/DiaCombatComponent.h"
#include "DiaComponent/DiaStatusEffectComponent.h"


#include "DiaInstance.h"
#include "Skill/DiaSkillBase.h"
#include "Skill/Active/DiaProjectileSkill.h"
#include "Skill/DiaSkillManager.h"
#include "Skill/DiaDamageCalculator.h"
#include "Skill/Effect/DiaStatusEffect.h"

#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "DiaBaseCharacter.h"

UDiaCombatComponent::UDiaCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

    CurrentCombatState = ECombatState::Idle;
    CombatTimer = 0.0f;
}


void UDiaCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 스킬 매니저 가져오기
	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		if (UDiaInstance* DiaInstance = Cast<UDiaInstance>(GameInstance))
		{
			SkillManager = DiaInstance->GetSkillManager();
		}
	}
	
	InitializeSkills();
	
	// 상태 이상 효과 컴포넌트 참조 가져오기
	UDiaStatusEffectComponent* StatusEffectComp = GetOwner()->FindComponentByClass<UDiaStatusEffectComponent>();
	if (IsValid(StatusEffectComp))
	{
		// 상태 이상 효과 추가/제거 시 이벤트 등록
		StatusEffectComp->OnStatusEffectAdded.AddDynamic(this, &UDiaCombatComponent::OnStatusEffectAdded);
		StatusEffectComp->OnStatusEffectRemoved.AddDynamic(this, &UDiaCombatComponent::OnStatusEffectRemoved);
	}
}

void UDiaCombatComponent::InitializeSkills()
{
	if (!IsValid(SkillManager)) return;

	// 기본 공격 스킬 등록
	const FSkillData* SkillData = SkillManager->GetSkillData(BasicAttackSkillID);
	if (SkillData == nullptr) return;

	RegisterSkill(BasicAttackSkillID);
	
}

void UDiaCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 전투 타이머 업데이트
    if (CurrentCombatState == ECombatState::InCombat)
    {
        UpdateCombatTimer(DeltaTime);
    }
}

void UDiaCombatComponent::SetCombatState(ECombatState NewState)
{
    if (NewState == CurrentCombatState)
    {
        return;
    }
    
    ECombatState OldState = CurrentCombatState;
    CurrentCombatState = NewState;
    
    // 상태 변경에 따른 처리
    switch (NewState)
    {
        case ECombatState::Idle:
            // 모든 전투 관련 타이머 초기화
            CombatTimer = 0.0f;
            break;
            
        case ECombatState::InCombat:
            // 전투 시작 처리
            CombatTimer = 0.0f;
            break;
            
        case ECombatState::Retreating:
            // 후퇴 처리
            break;
            
        case ECombatState::Dead:
            // 사망 처리
            HandleDeath();
            break;
    }
    
    // 이벤트 발생
    OnCombatStateChanged.Broadcast(NewState, OldState);
}

void UDiaCombatComponent::EnterCombat(AActor* CombatTarget)
{
    if (CurrentCombatState == ECombatState::Dead)
    {
        return;
    }
    
    SetCombatState(ECombatState::InCombat);
    
    if (IsValid(CombatTarget))
    {
        SetCurrentTarget(CombatTarget);
        AddThreatToActor(CombatTarget, 1.0f);
    }
    
    // 전투 시작 로그
    UE_LOG(LogTemp, Log, TEXT("%s entered combat with %s"), 
           *GetOwner()->GetName(), 
           IsValid(CombatTarget) ? *CombatTarget->GetName() : TEXT("unknown"));
}

float UDiaCombatComponent::ApplyDamage(AActor* Target, float BaseDamage, TSubclassOf<UDiaDamageType> DamageTypeClass)
{
    if (!IsValid(Target) || CurrentCombatState == ECombatState::Dead)
    {
        return 0.0f;
    }
    
    // 전투 상태로 변경
    EnterCombat(Target);
    
    // 기본 데미지 타입 설정
    TSubclassOf<UDiaDamageType> FinalDamageType = DamageTypeClass;
    if (!FinalDamageType)
    {
        FinalDamageType = TSubclassOf<UDiaDamageType>(UDiaDamageType::StaticClass());
    }
    
    // 데미지 계산 및 적용
    float ActualDamage = UDiaDamageCalculator::ApplyDamage(
        GetOwner(),
        Target,
        BaseDamage,
        FinalDamageType,
        0.05f // 기본 크리티컬 확률
    );
    
    // 이벤트 발생
    OnDamageDealt.Broadcast(ActualDamage, Target);
    
    // 위협도 증가
    AddThreatToActor(Target, ActualDamage);
    
    return ActualDamage;
}

void UDiaCombatComponent::ReceiveDamage(float DamageAmount, AActor* DamageCauser)
{
    if (CurrentCombatState == ECombatState::Dead)
    {
        return;
    }
    
    // 전투 상태로 변경
    EnterCombat(DamageCauser);
    
    // 데미지 적용
    CharacterData.Health = FMath::Max(0.0f, CharacterData.Health - DamageAmount);
    
    // 체력 변경 이벤트 발생
    OnHealthChanged.Broadcast(CharacterData.Health, CharacterData.MaxHealth);
    
    // 데미지 이벤트 발생
    OnDamageTaken.Broadcast(DamageAmount, DamageCauser);
    
    // 위협도 증가
    if (IsValid(DamageCauser))
    {
        AddThreatToActor(DamageCauser, DamageAmount * 2.0f); // 데미지의 2배만큼 위협도 증가
    }
    
    // 사망 체크
    if (CharacterData.Health <= 0.0f)
    {
        SetCombatState(ECombatState::Dead);
    }
}

void UDiaCombatComponent::HandleDeath()
{
    // 모든 스킬 중단
    for (auto& Pair : ActiveSkills)
    {
        if (IsValid(Pair.Value))
        {
            Pair.Value->EndSkill();
        }
    }
    
    // 타겟 초기화
    CurrentTarget = nullptr;
    
    // 위협 테이블 초기화
    ThreatTable.Empty();
    
    // 사망 이벤트 발생
    OnDeath.Broadcast();
    
    // 사망 로그
    UE_LOG(LogTemp, Log, TEXT("%s has died"), *GetOwner()->GetName());
}

void UDiaCombatComponent::SetCurrentTarget(AActor* NewTarget)
{
    if (CurrentTarget == NewTarget)
    {
        return;
    }
    
    CurrentTarget = NewTarget;
    
    // 타겟 변경 로그
    UE_LOG(LogTemp, Log, TEXT("%s changed target to %s"), 
           *GetOwner()->GetName(), 
           IsValid(NewTarget) ? *NewTarget->GetName() : TEXT("none"));
}


bool UDiaCombatComponent::ExecuteBasicAttack()
{
	return ExecuteSkill(BasicAttackSkillID);
}

bool UDiaCombatComponent::CanExecuteAttack() const
{
	return false;
}

float UDiaCombatComponent::CalculateDamage(float BaseDamage)
{
	// 기본 공격력 적용
	float FinalDamage = BaseDamage * (1.0f + CombatStats.AttackPower / 100.0f);
	
	// 크리티컬 확률 계산 (20% 기본 확률)
	bool bIsCritical = FMath::RandRange(0.0f, 1.0f) < 0.2f;
	if (bIsCritical)
	{
		// 크리티컬 시 데미지 1.5배
		FinalDamage *= 1.5f;
	}
	
	return FinalDamage;
}

// 스킬 비용 처리 함수 추가
bool UDiaCombatComponent::ProcessSkillCost(ADiaSkillBase* Skill)
{
    float ManaCost = Skill->GetManaCost();

    // 마나 부족 체크
    if (CharacterData.Mana < ManaCost)
    {
        return false;
    }

    // 마나 소모
    CharacterData.Mana -= ManaCost;

    return true;
}

bool UDiaCombatComponent::ExecuteSkill(int32 SkillID)
{
    ADiaSkillBase* Skill = ActiveSkills.FindRef(SkillID);
    if (!IsValid(Skill))
    {
        return false;
    }

    // 스킬 실행 가능 여부 확인 (쿨다운, 마나 등)
    if (!Skill->CanExecuteSkill())
    {
        return false;
    }
    
    // 스킬 비용 처리 (마나 등)
    if (!ProcessSkillCost(Skill))
    {
        return false;
    }
    
    // 스킬 실행
    Skill->ExecuteSkill();
    return true;
}

bool UDiaCombatComponent::RegisterSkill(int32 SkillID)
{
    // 이미 등록된 스킬인지 확인
    if (ActiveSkills.Contains(SkillID))
    {
        return true; // 이미 등록되어 있으면 성공으로 간주
    }
    
    // 스킬 매니저 유효성 검사
    if (!IsValid(SkillManager))
    {
        UE_LOG(LogTemp, Error, TEXT("SkillManager is not valid"));
        return false;
    }
    
    // 스킬 데이터 가져오기
    const FSkillData* SkillData = SkillManager->GetSkillData(SkillID);
    if (SkillData == nullptr || !IsValid(SkillData->SkillClass))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get skill data for ID: %d"), SkillID);
        return false;
    }
    
    // 스킬 생성
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner();
    
    ADiaSkillBase* NewSkill = GetWorld()->SpawnActor<ADiaSkillBase>(
        SkillData->SkillClass,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );
    
    // 스킬 초기화 및 등록
    if (IsValid(NewSkill))
    {
        NewSkill->InitializeWithData(SkillData);
        NewSkill->InitializeSkill(GetOwner());
        ActiveSkills.Add(SkillID, NewSkill);
        
        UE_LOG(LogTemp, Log, TEXT("Successfully registered skill ID: %d (%s)"), 
            SkillID, *SkillData->SkillName.ToString());
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Failed to spawn skill for ID: %d"), SkillID);
    return false;
}

void UDiaCombatComponent::UnregisterSkill(int32 SkillID)
{
	if (ActiveSkills.Contains(SkillID))
	{
		ActiveSkills[SkillID]->Destroy();
		ActiveSkills.Remove(SkillID);
	}	
}

void UDiaCombatComponent::AddThreatToActor(AActor* Actor, float ThreatAmount)
{
    if (!IsValid(Actor) || ThreatAmount <= 0.0f)
    {
        return;
    }
    
    // 기존 위협도에 추가
    float CurrentThreat = ThreatTable.FindRef(Actor);
    ThreatTable.Add(Actor, CurrentThreat + ThreatAmount);
    
    // 가장 높은 위협도를 가진 액터로 타겟 변경 (AI용)
    AActor* HighestThreatActor = GetHighestThreatActor();
    if (IsValid(HighestThreatActor) && HighestThreatActor != CurrentTarget)
    {
        // AI 캐릭터의 경우 자동으로 타겟 변경
        ADiaBaseCharacter* OwnerCharacter = Cast<ADiaBaseCharacter>(GetOwner());
        if (IsValid(OwnerCharacter) && !OwnerCharacter->IsPlayerControlled())
        {
            SetCurrentTarget(HighestThreatActor);
        }
    }
}

void UDiaCombatComponent::ClearAllThreats()
{
    ThreatTable.Empty();
}

AActor* UDiaCombatComponent::GetHighestThreatActor() const
{
    AActor* HighestThreatActor = nullptr;
    float HighestThreat = -1.0f;
    
    for (const auto& Pair : ThreatTable)
    {
        if (IsValid(Pair.Key) && Pair.Value > HighestThreat)
        {
            HighestThreatActor = Pair.Key;
            HighestThreat = Pair.Value;
        }
    }
    
    return HighestThreatActor;
}

void UDiaCombatComponent::UpdateCombatTimer(float DeltaTime)
{
    CombatTimer += DeltaTime;
    
    // 전투 타임아웃 체크
    if (CombatTimer >= CombatTimeout)
    {
        HandleCombatTimeout();
    }
}

void UDiaCombatComponent::HandleCombatTimeout()
{
    // 전투 종료 시간이 지나면 전투 상태 종료
    if (CurrentCombatState == ECombatState::InCombat)
    {
        UE_LOG(LogTemp, Log, TEXT("%s combat timed out"), *GetOwner()->GetName());
        ExitCombat();
    }
}

// 이벤트 핸들러 추가
void UDiaCombatComponent::OnStatusEffectAdded(UDiaStatusEffect* StatusEffect)
{
    if (!IsValid(StatusEffect))
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s gained status effect"), *GetOwner()->GetName());
    
    // 임시로 FName으로 직접 비교
    FName EffectTag = TEXT("Stun");
    if (StatusEffect->Tags.Contains(EffectTag))  // Tags는 UDiaStatusEffect에 추가해야 할 속성입니다
    {
        bCanAttack = false;
    }
}

void UDiaCombatComponent::OnStatusEffectRemoved(UDiaStatusEffect* StatusEffect)
{
    if (!IsValid(StatusEffect))
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("%s lost status effect"), *GetOwner()->GetName());
    
    // 임시로 FName으로 직접 비교
    FName EffectTag = TEXT("Stun");
    if (StatusEffect->Tags.Contains(EffectTag))  // Tags는 UDiaStatusEffect에 추가해야 할 속성입니다
    {
        bCanAttack = true;
    }
}

AActor* UDiaCombatComponent::FindNearestPlayer()
{
    AActor* NearestPlayer = nullptr;
    float NearestDistance = MAX_FLT;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<APlayerController> It(World); It; ++It)
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
            FVector MyLocation = GetOwner()->GetActorLocation();
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            float Distance = FVector::Distance(MyLocation, PlayerLocation);
            
            // 더 가까운 플레이어 업데이트
            if (Distance < NearestDistance)
            {
                NearestPlayer = PlayerPawn;
                NearestDistance = Distance;
            }
        }
    }
    
    return NearestPlayer;
}

void UDiaCombatComponent::ExitCombat()
{
    if (CurrentCombatState == ECombatState::Dead)
    {
        return;
    }

    // 전투 상태 변경
    SetCombatState(ECombatState::Idle);
    
    // 타겟 초기화
    CurrentTarget = nullptr;
    
    // 위협도 테이블 초기화
    ClearAllThreats();
    
    // 전투 타이머 초기화
    CombatTimer = 0.0f;
    
    // 로그 출력
    UE_LOG(LogTemp, Log, TEXT("%s exited combat"), *GetOwner()->GetName());
}
