// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Types/DiaCharacterTable.h"
#include "Dom/JsonObject.h"
#include "CharacterManager.generated.h"

class ADiaCharacter;

/**
 * 플레이어 캐릭터 데이터 관리 및 초기화를 담당하는 서브시스템
 * 캐릭터 테이블 로드, 캐릭터 초기화 등을 처리
 * JSON 파일과 데이터 테이블 모두 지원
 */
UCLASS()
class ARPG_API UCharacterManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 데이터 로드 방법 선택
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bUseJsonFiles = true; // true: JSON 파일 사용, false: 데이터 테이블 사용

	// 캐릭터 데이터 로드
	void LoadCharacterData();
	
	// JSON 파일 관련 함수
	void LoadCharacterDataFromJson();
	void SaveCharacterDataToJson();
	bool LoadSingleCharacterFromJson(const FString& FilePath, FCharacterInfo& OutCharacterInfo);
	bool SaveSingleCharacterToJson(const FString& FilePath, const FCharacterInfo& CharacterInfo);
	
	// JSON 변환 함수
	TSharedPtr<FJsonObject> CharacterInfoToJson(const FCharacterInfo& CharacterInfo);
	bool JsonToCharacterInfo(const TSharedPtr<FJsonObject>& JsonObject, FCharacterInfo& OutCharacterInfo);

	// 캐릭터 정보 가져오기
	const FCharacterInfo* GetCharacterInfo(FName CharacterID) const;
	const FCharacterInfo* GetCharacterInfoByClass(ECharacterClass CharacterClass) const;

	// 캐릭터 생성 및 초기화
	void InitializePlayerCharacter(ADiaCharacter* Character, const FCharacterCreationInfo& CreationInfo);
	void InitializePlayerCharacter(ADiaCharacter* Character, FName CharacterID, int32 Level = 1);

	// 레벨에 따른 스탯 계산
	float CalculateStatForLevel(const FCharacterInfo* CharacterInfo, float BaseStat, float PerLevelGrowth, int32 Level) const;
	float CalculateMaxHPForLevel(const FCharacterInfo* CharacterInfo, int32 Level) const;
	float CalculateMaxMPForLevel(const FCharacterInfo* CharacterInfo, int32 Level) const;
	float CalculateExpRequiredForLevel(const FCharacterInfo* CharacterInfo, int32 Level) const;

	// 사용 가능한 캐릭터 클래스 목록 가져오기
	TArray<ECharacterClass> GetAvailableCharacterClasses() const;

	// 캐시된 캐릭터 데이터 접근
	const TMap<FName, FCharacterInfo>& GetCharacterCache() const { return CharacterCache; }

	// 디폴트 캐릭터 ID 설정 (게임 시작시 사용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	FName DefaultCharacterID = TEXT("Warrior_Default");

	// JSON 파일 경로 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JSON Settings")
	FString JsonDataPath = TEXT("Data/Characters/");

protected:
	// 캐릭터 스탯 구성 요소별 초기화
	void InitializeCharacterStats(class UDiaStatComponent* StatComponent, const FCharacterInfo* CharacterInfo, int32 Level);
	void InitializeCombatStats(class UDiaStatComponent* StatComponent, const FCharacterInfo* CharacterInfo, int32 Level);
	void InitializeSkills(class ADiaCharacter* Character, const FCharacterInfo* CharacterInfo);

	// JSON 파일 유틸리티
	FString GetCharacterJsonFilePath(FName CharacterID) const;
	TArray<FString> GetAllCharacterJsonFiles() const;
	
	// 기본 캐릭터 생성
	void CreateDefaultCharacters();

private:
	// 데이터 테이블 경로
	UPROPERTY()
	FString CharacterDataTablePath = TEXT("/Game/Datatable/DT_DiaCharacterTable.DT_DiaCharacterTable");

	// 로드된 데이터 테이블
	UPROPERTY()
	UDataTable* CharacterDataTable;

	// 캐릭터 정보 캐시
	UPROPERTY()
	TMap<FName, FCharacterInfo> CharacterCache;
}; 