#include "System/GASSkillManager.h"
#include "GAS/Abilities/DiaBasicAttackAbility.h"
#include "GAS/Abilities/DiaMeleeAbility.h"
#include "GAS/Abilities/DiaProjectileAbility.h"
#include "Engine/Engine.h"
#include "Engine/DataTable.h"

void UGASSkillManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	// DataTable에서 로드 시도, 실패 시 기본 데이터 사용
	LoadSkillDataFromTable();
	
	if (SkillDataMap.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("GASSkillManager: DataTable load failed, using default data"));
		CreateDefaultSkillData();
	}
	
	UE_LOG(LogTemp, Log, TEXT("GASSkillManager: Initialized with %d skills"), SkillDataMap.Num());
}

const FGASSkillData* UGASSkillManager::GetSkillDataPtr(int32 SkillID) const
{
	if (const FGASSkillData* Found = SkillDataMap.Find(SkillID))
	{
		return Found;
	}
	return nullptr;
}

FGASSkillData UGASSkillManager::GetSkillData(int32 SkillID) const
{
	if (const FGASSkillData* Found = SkillDataMap.Find(SkillID))
	{
		return *Found;
	}
	return FGASSkillData{};
}

TArray<FGASSkillData> UGASSkillManager::GetAllSkillData() const
{
	TArray<FGASSkillData> AllSkills;
	for (const auto& Pair : SkillDataMap)
	{
		AllSkills.Add(Pair.Value);
	}
	return AllSkills;
}

TArray<FGASSkillData> UGASSkillManager::GetSkillsByType(EGASSkillType SkillType) const
{
	TArray<FGASSkillData> FilteredSkills;
	for (const auto& Pair : SkillDataMap)
	{
		if (Pair.Value.SkillType == SkillType)
		{
			FilteredSkills.Add(Pair.Value);
		}
	}
	return FilteredSkills;
}

void UGASSkillManager::CreateDefaultSkillData()
{
	// 1. 기본 공격 (Basic Attack)
	FGASSkillData BasicAttack;
	BasicAttack.SkillID = 1001;
	BasicAttack.SkillName = FText::FromString(TEXT("기본 공격"));
	BasicAttack.Description = FText::FromString(TEXT("기본적인 근접 공격을 합니다."));
	BasicAttack.AbilityClass = UDiaBasicAttackAbility::StaticClass();
	BasicAttack.SkillType = EGASSkillType::MeleeAttack;
	BasicAttack.TargetType = EGASTargetType::Enemy;
	BasicAttack.BaseDamage = 50.0f;
	BasicAttack.ManaCost = 0.0f;
	BasicAttack.CooldownDuration = 1.0f;
	BasicAttack.Range = 200.0f;
	BasicAttack.Radius = 50.0f;
	BasicAttack.RequiredCharacterLevel = 1;
	SkillDataMap.Add(BasicAttack.SkillID, BasicAttack);

	// 2. 강력한 공격 (Power Strike)
	FGASSkillData PowerStrike;
	PowerStrike.SkillID = 1002;
	PowerStrike.SkillName = FText::FromString(TEXT("강력한 공격"));
	PowerStrike.Description = FText::FromString(TEXT("마나를 소모하여 강력한 공격을 합니다."));
	PowerStrike.AbilityClass = UDiaMeleeAbility::StaticClass();
	PowerStrike.SkillType = EGASSkillType::MeleeAttack;
	PowerStrike.TargetType = EGASTargetType::Enemy;
	PowerStrike.BaseDamage = 100.0f;
	PowerStrike.ManaCost = 30.0f;
	PowerStrike.CooldownDuration = 3.0f;
	PowerStrike.CastTime = 0.5f;
	PowerStrike.Range = 250.0f;
	PowerStrike.Radius = 75.0f;
	PowerStrike.RequiredCharacterLevel = 3;
	SkillDataMap.Add(PowerStrike.SkillID, PowerStrike);

	// 3. 파이어볼트 (Fire Bolt)
	FGASSkillData FireBolt;
	FireBolt.SkillID = 1003;
	FireBolt.SkillName = FText::FromString(TEXT("파이어볼트"));
	FireBolt.Description = FText::FromString(TEXT("적에게 화염구를 발사합니다."));
	FireBolt.AbilityClass = UDiaProjectileAbility::StaticClass();
	FireBolt.SkillType = EGASSkillType::Magic;
	FireBolt.TargetType = EGASTargetType::Enemy;
	FireBolt.BaseDamage = 80.0f;
	FireBolt.ManaCost = 25.0f;
	FireBolt.CooldownDuration = 2.0f;
	FireBolt.CastTime = 1.0f;
	FireBolt.Range = 800.0f;
	FireBolt.Radius = 100.0f;
	FireBolt.RequiredCharacterLevel = 5;
	SkillDataMap.Add(FireBolt.SkillID, FireBolt);

	// 4. 힐 (Heal)
	FGASSkillData Heal;
	Heal.SkillID = 1004;
	Heal.SkillName = FText::FromString(TEXT("치유"));
	Heal.Description = FText::FromString(TEXT("자신의 체력을 회복합니다."));
	Heal.AbilityClass = UDiaGameplayAbility::StaticClass(); // 기본 클래스 사용
	Heal.SkillType = EGASSkillType::Heal;
	Heal.TargetType = EGASTargetType::Self;
	Heal.BaseDamage = -50.0f; // 음수 = 치유
	Heal.ManaCost = 40.0f;
	Heal.CooldownDuration = 5.0f;
	Heal.CastTime = 2.0f;
	Heal.Range = 0.0f;
	Heal.Radius = 0.0f;
	Heal.RequiredCharacterLevel = 2;
	SkillDataMap.Add(Heal.SkillID, Heal);

	// 5. 스프린트 (Sprint)
	FGASSkillData Sprint;
	Sprint.SkillID = 1005;
	Sprint.SkillName = FText::FromString(TEXT("질주"));
	Sprint.Description = FText::FromString(TEXT("짧은 시간 동안 이동속도를 증가시킵니다."));
	Sprint.AbilityClass = UDiaGameplayAbility::StaticClass();
	Sprint.SkillType = EGASSkillType::Buff;
	Sprint.TargetType = EGASTargetType::Self;
	Sprint.BaseDamage = 0.0f;
	Sprint.ManaCost = 20.0f;
	Sprint.CooldownDuration = 10.0f;
	Sprint.CastTime = 0.0f;
	Sprint.Range = 0.0f;
	Sprint.Radius = 0.0f;
	Sprint.RequiredCharacterLevel = 1;
	SkillDataMap.Add(Sprint.SkillID, Sprint);

	// 6. 멀티 샷 (Multi Shot)
	FGASSkillData MultiShot;
	MultiShot.SkillID = 1006;
	MultiShot.SkillName = FText::FromString(TEXT("다중 사격"));
	MultiShot.Description = FText::FromString(TEXT("여러 개의 투사체를 동시에 발사합니다."));
	MultiShot.AbilityClass = UDiaProjectileAbility::StaticClass();
	MultiShot.SkillType = EGASSkillType::RangedAttack;
	MultiShot.TargetType = EGASTargetType::Direction;
	MultiShot.BaseDamage = 40.0f;
	MultiShot.ManaCost = 35.0f;
	MultiShot.CooldownDuration = 4.0f;
	MultiShot.CastTime = 0.8f;
	MultiShot.Range = 600.0f;
	MultiShot.Radius = 150.0f;
	MultiShot.RequiredCharacterLevel = 7;
	SkillDataMap.Add(MultiShot.SkillID, MultiShot);

	// 7. 어스 스파이크 (Earth Spike)
	FGASSkillData EarthSpike;
	EarthSpike.SkillID = 1007;
	EarthSpike.SkillName = FText::FromString(TEXT("대지 창"));
	EarthSpike.Description = FText::FromString(TEXT("지면에서 바위 창을 소환합니다."));
	EarthSpike.AbilityClass = UDiaGameplayAbility::StaticClass();
	EarthSpike.SkillType = EGASSkillType::Magic;
	EarthSpike.TargetType = EGASTargetType::Ground;
	EarthSpike.BaseDamage = 120.0f;
	EarthSpike.ManaCost = 45.0f;
	EarthSpike.CooldownDuration = 6.0f;
	EarthSpike.CastTime = 1.5f;
	EarthSpike.Range = 500.0f;
	EarthSpike.Radius = 200.0f;
	EarthSpike.RequiredCharacterLevel = 10;
	SkillDataMap.Add(EarthSpike.SkillID, EarthSpike);

	UE_LOG(LogTemp, Log, TEXT("GASSkillManager: Created %d default skills"), SkillDataMap.Num());
}

void UGASSkillManager::LoadSkillDataFromTable()
{
	// DataTable 경로 설정 (프로젝트 설정에서 변경 가능)
	if (!SkillDataTable.IsValid())
	{
		// 기본 DataTable 경로 설정
		SkillDataTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Datatable/DT_GASSkillData.DT_GASSkillData")));
	}

	UDataTable* DataTable = SkillDataTable.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GASSkillManager: Failed to load DataTable at path: %s"), 
			*SkillDataTable.ToString());
		return;
	}

	// DataTable 구조체 검증
	if (DataTable->GetRowStruct() != FGASSkillData::StaticStruct())
	{
		UE_LOG(LogTemp, Error, TEXT("GASSkillManager: DataTable row struct mismatch!"));
		return;
	}

	// 모든 행 가져오기
	TArray<FName> RowNames = DataTable->GetRowNames();
	SkillDataMap.Empty();

	for (const FName& RowName : RowNames)
	{
		FGASSkillData* RowData = DataTable->FindRow<FGASSkillData>(RowName, TEXT("GASSkillManager"));
		if (RowData)
		{
			SkillDataMap.Add(RowData->SkillID, *RowData);
			UE_LOG(LogTemp, Log, TEXT("GASSkillManager: Loaded skill %d: %s"), 
				RowData->SkillID, *RowData->SkillName.ToString());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("GASSkillManager: Loaded %d skills from DataTable"), SkillDataMap.Num());
}