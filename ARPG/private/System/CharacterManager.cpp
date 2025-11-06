// Fill out your copyright notice in the Description page of Project Settings.

#include "System/CharacterManager.h"
#include "Character/DiaCharacter.h"

#include "Engine/DataTable.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UCharacterManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadCharacterData();
}

void UCharacterManager::Deinitialize()
{
	Super::Deinitialize();
	CharacterDataTable = nullptr;
	CharacterCache.Empty();
}

void UCharacterManager::LoadCharacterData()
{
	if (bUseJsonFiles)
	{
		LoadCharacterDataFromJson();
	}
	else
	{
		// 기존 데이터 테이블 로딩 방식
		CharacterDataTable = LoadObject<UDataTable>(nullptr, *CharacterDataTablePath);
		if (CharacterDataTable)
		{
			CharacterCache.Empty();
			TArray<FName> RowNames = CharacterDataTable->GetRowNames();
			for (const FName& RowName : RowNames)
			{
				FCharacterInfo* CharacterRow = CharacterDataTable->FindRow<FCharacterInfo>(RowName, TEXT(""));
				if (CharacterRow)
				{
					CharacterCache.Emplace(CharacterRow->CharacterID, *CharacterRow);
					UE_LOG(LogTemp, Log, TEXT("CharacterManager: 캐릭터 데이터 로드됨 - ID: %s, 클래스: %s"), 
						*CharacterRow->CharacterID.ToString(), 
						*UEnum::GetValueAsString(CharacterRow->CharacterClass));
				}
			}
			
			UE_LOG(LogTemp, Log, TEXT("CharacterManager: 총 %d개 캐릭터 데이터 로드 완료 (DataTable)"), CharacterCache.Num());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CharacterManager: 캐릭터 데이터 테이블 로드 실패 - 경로: %s"), *CharacterDataTablePath);
		}
	}
}

const FCharacterInfo* UCharacterManager::GetCharacterInfo(FName CharacterID) const
{
	if (const FCharacterInfo* CharacterInfo = CharacterCache.Find(CharacterID))
	{
		return CharacterInfo;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("CharacterManager: 캐릭터 정보를 찾을 수 없음 - ID: %s"), *CharacterID.ToString());
	return nullptr;
}

const FCharacterInfo* UCharacterManager::GetCharacterInfoByClass(ECharacterClass CharacterClass) const
{
	for (const auto& Pair : CharacterCache)
	{
		if (Pair.Value.CharacterClass == CharacterClass)
		{
			return &Pair.Value;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("CharacterManager: 해당 클래스의 캐릭터 정보를 찾을 수 없음 - 클래스: %s"), 
		*UEnum::GetValueAsString(CharacterClass));
	return nullptr;
}

void UCharacterManager::InitializePlayerCharacter(ADiaCharacter* Character, const FCharacterCreationInfo& CreationInfo)
{
	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterManager: 유효하지 않은 캐릭터"));
		return;
	}

	const FCharacterInfo* CharacterInfo = GetCharacterInfo(CreationInfo.CharacterID);
	if (!CharacterInfo)
	{
		// 대체 캐릭터로 기본 전사 사용
		CharacterInfo = GetCharacterInfoByClass(ECharacterClass::Warrior);
		if (!CharacterInfo)
		{
			UE_LOG(LogTemp, Error, TEXT("CharacterManager: 기본 캐릭터 정보도 찾을 수 없음"));
			return;
		}
	}

	InitializePlayerCharacter(Character, CharacterInfo->CharacterID, CreationInfo.StartLevel);
	
	UE_LOG(LogTemp, Log, TEXT("CharacterManager: 플레이어 캐릭터 초기화 완료 - %s (레벨 %d)"), 
		*CreationInfo.PlayerName, CreationInfo.StartLevel);
}

//초기화
void UCharacterManager::InitializePlayerCharacter(ADiaCharacter* Character, FName CharacterID, int32 Level)
{
	if (!IsValid(Character))
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterManager: 유효하지 않은 캐릭터"));
		return;
	}

	const FCharacterInfo* CharacterInfo = GetCharacterInfo(CharacterID);
	if (!CharacterInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterManager: 캐릭터 정보를 찾을 수 없음 - ID: %s"), *CharacterID.ToString());
		return;
	}

	// 스킬 초기화
	InitializeSkills(Character, CharacterInfo);

	// 외형 설정
	if (CharacterInfo->CharacterMesh.IsValid())
	{
		USkeletalMesh* CharacterMeshAsset = CharacterInfo->CharacterMesh.LoadSynchronous();
		if (CharacterMeshAsset && Character->GetMesh())
		{
			Character->GetMesh()->SetSkeletalMesh(CharacterMeshAsset);
			Character->GetMesh()->RecreateRenderState_Concurrent();
		}
	}

	// 애니메이션 블루프린트 설정
	if (CharacterInfo->AnimationBlueprint && Character->GetMesh())
	{
		Character->GetMesh()->SetAnimInstanceClass(CharacterInfo->AnimationBlueprint);
	}

	UE_LOG(LogTemp, Log, TEXT("CharacterManager: 캐릭터 초기화 완료 - ID: %s, 레벨: %d"), 
		*CharacterID.ToString(), Level);
}



void UCharacterManager::InitializeSkills(ADiaCharacter* Character, const FCharacterInfo* CharacterInfo)
{
	if (!IsValid(Character) || !CharacterInfo)
	{
		return;
	}

}

float UCharacterManager::CalculateStatForLevel(const FCharacterInfo* CharacterInfo, float BaseStat, float PerLevelGrowth, int32 Level) const
{
	if (!CharacterInfo)
	{
		return BaseStat;
	}

	// 기본 스탯 + (레벨-1) * 레벨당 증가량
	return BaseStat + (Level - 1) * PerLevelGrowth;
}

float UCharacterManager::CalculateMaxHPForLevel(const FCharacterInfo* CharacterInfo, int32 Level) const
{
	return CalculateStatForLevel(CharacterInfo, CharacterInfo->BaseMaxHP, CharacterInfo->HPPerLevel, Level);
}

float UCharacterManager::CalculateMaxMPForLevel(const FCharacterInfo* CharacterInfo, int32 Level) const
{
	return CalculateStatForLevel(CharacterInfo, CharacterInfo->BaseMaxMP, CharacterInfo->MPPerLevel, Level);
}

float UCharacterManager::CalculateExpRequiredForLevel(const FCharacterInfo* CharacterInfo, int32 Level) const
{
	if (!CharacterInfo)
	{
		return 100.0f;
	}

	// 지수적 증가: 기본 경험치 * (증가율 ^ (레벨-1))
	return CharacterInfo->BaseExpRequired * FMath::Pow(CharacterInfo->ExpGrowthRate, Level - 1);
}

TArray<ECharacterClass> UCharacterManager::GetAvailableCharacterClasses() const
{
	TArray<ECharacterClass> AvailableClasses;
	TSet<ECharacterClass> UniqueClasses;

	for (const auto& Pair : CharacterCache)
	{
		if (Pair.Value.CharacterClass != ECharacterClass::None)
		{
			UniqueClasses.Add(Pair.Value.CharacterClass);
		}
	}

	AvailableClasses = UniqueClasses.Array();
	return AvailableClasses;
}

//////////////////////////////////////////////////////////////////////////
// JSON 파일 관련 함수 구현
//////////////////////////////////////////////////////////////////////////

void UCharacterManager::LoadCharacterDataFromJson()
{
	CharacterCache.Empty();
	
	TArray<FString> JsonFiles = GetAllCharacterJsonFiles();
	
	if (JsonFiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterManager: JSON 파일이 없습니다. 기본 캐릭터 생성"));
		// 기본 캐릭터 생성 및 저장
		CreateDefaultCharacters();
		return;
	}
	
	for (const FString& FilePath : JsonFiles)
	{
		FCharacterInfo CharacterInfo;
		if (LoadSingleCharacterFromJson(FilePath, CharacterInfo))
		{
			CharacterCache.Emplace(CharacterInfo.CharacterID, CharacterInfo);
			UE_LOG(LogTemp, Log, TEXT("CharacterManager: JSON에서 캐릭터 데이터 로드됨 - ID: %s, 클래스: %s"), 
				*CharacterInfo.CharacterID.ToString(), 
				*UEnum::GetValueAsString(CharacterInfo.CharacterClass));
		}
	}
	
	UE_LOG(LogTemp, Log, TEXT("CharacterManager: 총 %d개 캐릭터 데이터 로드 완료 (JSON)"), CharacterCache.Num());
}

void UCharacterManager::SaveCharacterDataToJson()
{
	for (const auto& Pair : CharacterCache)
	{
		FString FilePath = GetCharacterJsonFilePath(Pair.Key);
		SaveSingleCharacterToJson(FilePath, Pair.Value);
	}
	
	UE_LOG(LogTemp, Log, TEXT("CharacterManager: 모든 캐릭터 데이터를 JSON으로 저장 완료"));
}

bool UCharacterManager::LoadSingleCharacterFromJson(const FString& FilePath, FCharacterInfo& OutCharacterInfo)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterManager: JSON 파일 로드 실패 - %s"), *FilePath);
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterManager: JSON 파싱 실패 - %s"), *FilePath);
		return false;
	}

	return JsonToCharacterInfo(JsonObject, OutCharacterInfo);
}

bool UCharacterManager::SaveSingleCharacterToJson(const FString& FilePath, const FCharacterInfo& CharacterInfo)
{
	TSharedPtr<FJsonObject> JsonObject = CharacterInfoToJson(CharacterInfo);
	if (!JsonObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterManager: JSON 변환 실패"));
		return false;
	}

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterManager: JSON 직렬화 실패"));
		return false;
	}

	// 디렉토리 생성
	FString Directory = FPaths::GetPath(FilePath);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*Directory))
	{
		PlatformFile.CreateDirectoryTree(*Directory);
	}

	if (!FFileHelper::SaveStringToFile(OutputString, *FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterManager: JSON 파일 저장 실패 - %s"), *FilePath);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("CharacterManager: JSON 파일 저장 성공 - %s"), *FilePath);
	return true;
}

TSharedPtr<FJsonObject> UCharacterManager::CharacterInfoToJson(const FCharacterInfo& CharacterInfo)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// 기본 정보
	JsonObject->SetStringField(TEXT("CharacterID"), CharacterInfo.CharacterID.ToString());
	JsonObject->SetStringField(TEXT("Name"), CharacterInfo.Name.ToString());
	JsonObject->SetNumberField(TEXT("CharacterClass"), (int32)CharacterInfo.CharacterClass);
	JsonObject->SetNumberField(TEXT("StartLevel"), CharacterInfo.StartLevel);

	// 체력 관련
	JsonObject->SetNumberField(TEXT("BaseMaxHP"), CharacterInfo.BaseMaxHP);
	JsonObject->SetNumberField(TEXT("HPPerLevel"), CharacterInfo.HPPerLevel);

	// 마나 관련
	JsonObject->SetNumberField(TEXT("BaseMaxMP"), CharacterInfo.BaseMaxMP);
	JsonObject->SetNumberField(TEXT("MPPerLevel"), CharacterInfo.MPPerLevel);

	// 기본 스탯
	JsonObject->SetNumberField(TEXT("BaseStrength"), CharacterInfo.BaseStrength);
	JsonObject->SetNumberField(TEXT("BaseIntelligence"), CharacterInfo.BaseIntelligence);
	JsonObject->SetNumberField(TEXT("BaseDexterity"), CharacterInfo.BaseDexterity);
	JsonObject->SetNumberField(TEXT("BaseConstitution"), CharacterInfo.BaseConstitution);

	// 레벨당 스탯 증가량
	JsonObject->SetNumberField(TEXT("StrengthPerLevel"), CharacterInfo.StrengthPerLevel);
	JsonObject->SetNumberField(TEXT("IntelligencePerLevel"), CharacterInfo.IntelligencePerLevel);
	JsonObject->SetNumberField(TEXT("DexterityPerLevel"), CharacterInfo.DexterityPerLevel);
	JsonObject->SetNumberField(TEXT("ConstitutionPerLevel"), CharacterInfo.ConstitutionPerLevel);

	// 전투 스탯
	JsonObject->SetNumberField(TEXT("BaseAttackPower"), CharacterInfo.BaseAttackPower);
	JsonObject->SetNumberField(TEXT("BaseDefense"), CharacterInfo.BaseDefense);
	JsonObject->SetNumberField(TEXT("BaseAttackSpeed"), CharacterInfo.BaseAttackSpeed);
	JsonObject->SetNumberField(TEXT("BaseAttackRange"), CharacterInfo.BaseAttackRange);

	// 전투 스탯 레벨당 증가량
	JsonObject->SetNumberField(TEXT("AttackPowerPerLevel"), CharacterInfo.AttackPowerPerLevel);
	JsonObject->SetNumberField(TEXT("DefensePerLevel"), CharacterInfo.DefensePerLevel);

	// 경험치 관련
	JsonObject->SetNumberField(TEXT("BaseExpRequired"), CharacterInfo.BaseExpRequired);
	JsonObject->SetNumberField(TEXT("ExpGrowthRate"), CharacterInfo.ExpGrowthRate);

	// 초기 스킬 배열
	TArray<TSharedPtr<FJsonValue>> SkillArray;
	for (int32 SkillID : CharacterInfo.InitialSkillIDs)
	{
		SkillArray.Add(MakeShareable(new FJsonValueNumber(SkillID)));
	}
	JsonObject->SetArrayField(TEXT("InitialSkillIDs"), SkillArray);

	// 외형 정보
	if (CharacterInfo.CharacterMesh.IsValid())
	{
		JsonObject->SetStringField(TEXT("CharacterMesh"), CharacterInfo.CharacterMesh.ToString());
	}

	return JsonObject;
}

bool UCharacterManager::JsonToCharacterInfo(const TSharedPtr<FJsonObject>& JsonObject, FCharacterInfo& OutCharacterInfo)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	// 기본 정보
	FString CharacterIDString;
	if (JsonObject->TryGetStringField(TEXT("CharacterID"), CharacterIDString))
	{
		OutCharacterInfo.CharacterID = FName(*CharacterIDString);
	}

	FString NameString;
	if (JsonObject->TryGetStringField(TEXT("Name"), NameString))
	{
		OutCharacterInfo.Name = FText::FromString(NameString);
	}

	int32 CharacterClassInt;
	if (JsonObject->TryGetNumberField(TEXT("CharacterClass"), CharacterClassInt))
	{
		OutCharacterInfo.CharacterClass = (ECharacterClass)CharacterClassInt;
	}

	JsonObject->TryGetNumberField(TEXT("StartLevel"), OutCharacterInfo.StartLevel);

	// 체력 관련
	JsonObject->TryGetNumberField(TEXT("BaseMaxHP"), OutCharacterInfo.BaseMaxHP);
	JsonObject->TryGetNumberField(TEXT("HPPerLevel"), OutCharacterInfo.HPPerLevel);

	// 마나 관련
	JsonObject->TryGetNumberField(TEXT("BaseMaxMP"), OutCharacterInfo.BaseMaxMP);
	JsonObject->TryGetNumberField(TEXT("MPPerLevel"), OutCharacterInfo.MPPerLevel);

	// 기본 스탯
	JsonObject->TryGetNumberField(TEXT("BaseStrength"), OutCharacterInfo.BaseStrength);
	JsonObject->TryGetNumberField(TEXT("BaseIntelligence"), OutCharacterInfo.BaseIntelligence);
	JsonObject->TryGetNumberField(TEXT("BaseDexterity"), OutCharacterInfo.BaseDexterity);
	JsonObject->TryGetNumberField(TEXT("BaseConstitution"), OutCharacterInfo.BaseConstitution);

	// 레벨당 스탯 증가량
	JsonObject->TryGetNumberField(TEXT("StrengthPerLevel"), OutCharacterInfo.StrengthPerLevel);
	JsonObject->TryGetNumberField(TEXT("IntelligencePerLevel"), OutCharacterInfo.IntelligencePerLevel);
	JsonObject->TryGetNumberField(TEXT("DexterityPerLevel"), OutCharacterInfo.DexterityPerLevel);
	JsonObject->TryGetNumberField(TEXT("ConstitutionPerLevel"), OutCharacterInfo.ConstitutionPerLevel);

	// 전투 스탯
	JsonObject->TryGetNumberField(TEXT("BaseAttackPower"), OutCharacterInfo.BaseAttackPower);
	JsonObject->TryGetNumberField(TEXT("BaseDefense"), OutCharacterInfo.BaseDefense);
	JsonObject->TryGetNumberField(TEXT("BaseAttackSpeed"), OutCharacterInfo.BaseAttackSpeed);
	JsonObject->TryGetNumberField(TEXT("BaseAttackRange"), OutCharacterInfo.BaseAttackRange);

	// 전투 스탯 레벨당 증가량
	JsonObject->TryGetNumberField(TEXT("AttackPowerPerLevel"), OutCharacterInfo.AttackPowerPerLevel);
	JsonObject->TryGetNumberField(TEXT("DefensePerLevel"), OutCharacterInfo.DefensePerLevel);

	// 경험치 관련
	JsonObject->TryGetNumberField(TEXT("BaseExpRequired"), OutCharacterInfo.BaseExpRequired);
	JsonObject->TryGetNumberField(TEXT("ExpGrowthRate"), OutCharacterInfo.ExpGrowthRate);

	// 초기 스킬 배열
	const TArray<TSharedPtr<FJsonValue>>* SkillArray;
	if (JsonObject->TryGetArrayField(TEXT("InitialSkillIDs"), SkillArray))
	{
		OutCharacterInfo.InitialSkillIDs.Empty();
		for (const TSharedPtr<FJsonValue>& Value : *SkillArray)
		{
			int32 SkillID;
			if (Value->TryGetNumber(SkillID))
			{
				OutCharacterInfo.InitialSkillIDs.Add(SkillID);
			}
		}
	}

	// 외형 정보
	FString MeshPath;
	if (JsonObject->TryGetStringField(TEXT("CharacterMesh"), MeshPath))
	{
		OutCharacterInfo.CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(*MeshPath));
	}

	return true;
}

FString UCharacterManager::GetCharacterJsonFilePath(FName CharacterID) const
{
	FString ContentDir = FPaths::ProjectContentDir();
	return FPaths::Combine(ContentDir, JsonDataPath, CharacterID.ToString() + TEXT(".json"));
}

TArray<FString> UCharacterManager::GetAllCharacterJsonFiles() const
{
	TArray<FString> JsonFiles;
	FString SearchPath = FPaths::Combine(FPaths::ProjectContentDir(), JsonDataPath);
	
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	
	if (PlatformFile.DirectoryExists(*SearchPath))
	{
		TArray<FString> FoundFiles;
		PlatformFile.FindFilesRecursively(FoundFiles, *SearchPath, TEXT("*.json"));
		
		// FindFilesRecursively는 전체 경로를 반환하므로 그대로 사용
		JsonFiles = FoundFiles;
		
		UE_LOG(LogTemp, Log, TEXT("CharacterManager: JSON 파일 검색 경로: %s"), *SearchPath);
		for (const FString& FilePath : JsonFiles)
		{
			UE_LOG(LogTemp, Log, TEXT("CharacterManager: 발견된 JSON 파일: %s"), *FilePath);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterManager: JSON 디렉토리가 존재하지 않음: %s"), *SearchPath);
	}
	
	return JsonFiles;
}

void UCharacterManager::CreateDefaultCharacters()
{
	// 기본 전사 캐릭터 생성
	FCharacterInfo WarriorInfo;
	WarriorInfo.CharacterID = TEXT("Warrior_Default");
	WarriorInfo.Name = FText::FromString(TEXT("전사"));
	WarriorInfo.CharacterClass = ECharacterClass::Warrior;
	WarriorInfo.StartLevel = 1;
	WarriorInfo.BaseMaxHP = 120.0f;
	WarriorInfo.HPPerLevel = 15.0f;
	WarriorInfo.BaseMaxMP = 30.0f;
	WarriorInfo.MPPerLevel = 3.0f;
	WarriorInfo.BaseStrength = 15.0f;
	WarriorInfo.BaseIntelligence = 8.0f;
	WarriorInfo.BaseDexterity = 10.0f;
	WarriorInfo.BaseConstitution = 14.0f;
	WarriorInfo.StrengthPerLevel = 2.0f;
	WarriorInfo.IntelligencePerLevel = 1.0f;
	WarriorInfo.DexterityPerLevel = 1.5f;
	WarriorInfo.ConstitutionPerLevel = 2.5f;
	WarriorInfo.BaseAttackPower = 15.0f;
	WarriorInfo.BaseDefense = 8.0f;
	WarriorInfo.BaseAttackSpeed = 1.0f;
	WarriorInfo.BaseAttackRange = 150.0f;
	WarriorInfo.AttackPowerPerLevel = 3.0f;
	WarriorInfo.DefensePerLevel = 1.5f;
	WarriorInfo.BaseExpRequired = 100.0f;
	WarriorInfo.ExpGrowthRate = 1.4f;
	WarriorInfo.InitialSkillIDs = {1, 2, 3};

	// 기본 마법사 캐릭터 생성
	FCharacterInfo MageInfo;
	MageInfo.CharacterID = TEXT("Mage_Default");
	MageInfo.Name = FText::FromString(TEXT("마법사"));
	MageInfo.CharacterClass = ECharacterClass::Mage;
	MageInfo.StartLevel = 1;
	MageInfo.BaseMaxHP = 80.0f;
	MageInfo.HPPerLevel = 10.0f;
	MageInfo.BaseMaxMP = 80.0f;
	MageInfo.MPPerLevel = 8.0f;
	MageInfo.BaseStrength = 8.0f;
	MageInfo.BaseIntelligence = 18.0f;
	MageInfo.BaseDexterity = 12.0f;
	MageInfo.BaseConstitution = 10.0f;
	MageInfo.StrengthPerLevel = 1.0f;
	MageInfo.IntelligencePerLevel = 3.0f;
	MageInfo.DexterityPerLevel = 2.0f;
	MageInfo.ConstitutionPerLevel = 1.5f;
	MageInfo.BaseAttackPower = 12.0f;
	MageInfo.BaseDefense = 5.0f;
	MageInfo.BaseAttackSpeed = 1.0f;
	MageInfo.BaseAttackRange = 200.0f;
	MageInfo.AttackPowerPerLevel = 2.5f;
	MageInfo.DefensePerLevel = 1.0f;
	MageInfo.BaseExpRequired = 100.0f;
	MageInfo.ExpGrowthRate = 1.4f;
	MageInfo.InitialSkillIDs = {4, 5, 6};

	// 기본 궁수 캐릭터 생성
	FCharacterInfo ArcherInfo;
	ArcherInfo.CharacterID = TEXT("Archer_Default");
	ArcherInfo.Name = FText::FromString(TEXT("궁수"));
	ArcherInfo.CharacterClass = ECharacterClass::Archer;
	ArcherInfo.StartLevel = 1;
	ArcherInfo.BaseMaxHP = 100.0f;
	ArcherInfo.HPPerLevel = 12.0f;
	ArcherInfo.BaseMaxMP = 50.0f;
	ArcherInfo.MPPerLevel = 5.0f;
	ArcherInfo.BaseStrength = 12.0f;
	ArcherInfo.BaseIntelligence = 12.0f;
	ArcherInfo.BaseDexterity = 18.0f;
	ArcherInfo.BaseConstitution = 8.0f;
	ArcherInfo.StrengthPerLevel = 1.5f;
	ArcherInfo.IntelligencePerLevel = 2.0f;
	ArcherInfo.DexterityPerLevel = 3.0f;
	ArcherInfo.ConstitutionPerLevel = 1.0f;
	ArcherInfo.BaseAttackPower = 14.0f;
	ArcherInfo.BaseDefense = 6.0f;
	ArcherInfo.BaseAttackSpeed = 1.2f;
	ArcherInfo.BaseAttackRange = 300.0f;
	ArcherInfo.AttackPowerPerLevel = 2.8f;
	ArcherInfo.DefensePerLevel = 1.2f;
	ArcherInfo.BaseExpRequired = 100.0f;
	ArcherInfo.ExpGrowthRate = 1.4f;
	ArcherInfo.InitialSkillIDs = {7, 8, 9};

	// 캐시에 추가
	CharacterCache.Emplace(WarriorInfo.CharacterID, WarriorInfo);
	CharacterCache.Emplace(MageInfo.CharacterID, MageInfo);
	CharacterCache.Emplace(ArcherInfo.CharacterID, ArcherInfo);

	// JSON 파일로 저장
	SaveCharacterDataToJson();

	UE_LOG(LogTemp, Log, TEXT("CharacterManager: 기본 캐릭터 3개 생성 및 저장 완료"));
} 