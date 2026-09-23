#include "Controller/DiaController.h"

#include "DiaBaseCharacter.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "Engine/GameInstance.h"
#include "GAS/DiaGameplayTags.h"
#include "Logging/ARPGLogChannels.h"
#include "System/GASSkillManager.h"

#if !UE_BUILD_SHIPPING
namespace
{
	void ReportSkillCheat(ADiaController* Controller, const FString& Message)
	{
		UE_LOG(LogARPG, Display, TEXT("[SkillCheat] %s"), *Message);
		Controller->ClientMessage(Message);
	}
}
#endif

void ADiaController::CheatGiveSkill(int32 SkillID, int32 QuickSlot)
{
#if UE_BUILD_SHIPPING
	return;
#else
	if (SkillID <= 0 || QuickSlot < 0 || QuickSlot > UDiaSkillManagerComponent::MaxQuickSlotCount)
	{
		ReportSkillCheat(this, TEXT("사용법: CheatGiveSkill 스킬ID [슬롯 1~5]. 슬롯 생략 또는 0: 자동 배치."));
		return;
	}

	ADiaBaseCharacter* DiaCharacter = Cast<ADiaBaseCharacter>(GetPawn());
	UDiaSkillManagerComponent* Skills = IsValid(DiaCharacter) ? DiaCharacter->GetSkillManagerComponent() : nullptr;
	if (!IsValid(DiaCharacter) || !IsValid(Skills))
	{
		ReportSkillCheat(this, TEXT("플레이를 시작하고 캐릭터를 조종한 상태에서 실행하세요."));
		return;
	}
	if (!DiaCharacter->HasAuthority())
	{
		ReportSkillCheat(this, TEXT("스킬 지급은 단독 플레이 또는 서버 플레이어에서 실행하세요."));
		return;
	}

	UGASSkillManager* Data = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
	const FGASSkillData* Row = IsValid(Data) ? Data->GetSkillDataPtr(SkillID) : nullptr;
	if (!Row)
	{
		ReportSkillCheat(this, FString::Printf(TEXT("로드된 스킬 ID %d가 없습니다. CheatListSkills로 확인하세요."), SkillID));
		return;
	}

	const bool bOwned = Skills->GetSkillObjectBySkillID(SkillID) != nullptr;
	if (!bOwned && !Skills->HasAvailableSkillSlot())
	{
		ReportSkillCheat(this, FString::Printf(TEXT("보유 스킬이 가득 찼습니다. 최대 %d개입니다."),
			UDiaSkillManagerComponent::MaxOwnedSkillCount));
		return;
	}
	if ((!bOwned || !Skills->GetAbilitySpecBySkillID(SkillID)) && !DiaCharacter->ApplySkillByID(SkillID))
	{
		ReportSkillCheat(this, TEXT("스킬 지급에 실패했습니다. 캐릭터의 GAS 초기화와 스킬 AbilityClass를 확인하세요."));
		return;
	}

	if (QuickSlot > 0)
	{
		// Move this skill without leaving duplicate bindings; the displaced skill stays owned.
		const int32 OldSlot = Skills->GetIndexOfSkillID(SkillID);
		if (OldSlot != INDEX_NONE && OldSlot != QuickSlot - 1)
		{
			DiaCharacter->ClearSkillFromQuickSlot(SkillID);
		}
		DiaCharacter->SetSkillIDOnQuickSlotWidget(SkillID, QuickSlot - 1);
	}
	else
	{
		DiaCharacter->TryAutoAssignSkillToQuickSlot(SkillID);
	}

	const int32 Slot = Skills->GetIndexOfSkillID(SkillID);
	const FString Result = Slot == INDEX_NONE
		? TEXT("보유 중. 빈 퀵슬롯이 없습니다. 슬롯 1~5를 지정해 다시 실행하세요.")
		: FString::Printf(TEXT("퀵슬롯 %d에 배치했습니다."), Slot + 1);
	ReportSkillCheat(this, FString::Printf(TEXT("%s (%d): %s"), *Row->SkillName.ToString(), SkillID, *Result));
#endif
}

void ADiaController::CheatGiveAllSkills(bool bIncludeNonPlayerSkills)
{
#if UE_BUILD_SHIPPING
	return;
#else
	ADiaBaseCharacter* DiaCharacter = Cast<ADiaBaseCharacter>(GetPawn());
	UDiaSkillManagerComponent* Skills = IsValid(DiaCharacter) ? DiaCharacter->GetSkillManagerComponent() : nullptr;
	if (!IsValid(DiaCharacter) || !IsValid(Skills))
	{
		ReportSkillCheat(this, TEXT("플레이를 시작하고 캐릭터를 조종한 상태에서 실행하세요."));
		return;
	}
	if (!DiaCharacter->HasAuthority())
	{
		ReportSkillCheat(this, TEXT("스킬 지급은 단독 플레이 또는 서버 플레이어에서 실행하세요."));
		return;
	}
	UGASSkillManager* Data = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
	if (!IsValid(Data))
	{
		ReportSkillCheat(this, TEXT("스킬 데이터가 없습니다. 플레이를 시작한 뒤 실행하세요."));
		return;
	}
	const FGameplayTag PlayerSkillTag = FDiaGameplayTags::Get().SkillType_Character_Player;
	if (!bIncludeNonPlayerSkills && !PlayerSkillTag.IsValid())
	{
		ReportSkillCheat(this, TEXT("플레이어 스킬 태그가 초기화되지 않았습니다."));
		return;
	}

	TArray<FGASSkillData> Rows = Data->GetAllSkillData();
	Rows.Sort([](const FGASSkillData& A, const FGASSkillData& B) { return A.SkillID < B.SkillID; });
	int32 GrantedCount = 0;
	int32 AlreadyOwnedCount = 0;
	int32 FailedCount = 0;
	for (const FGASSkillData& Row : Rows)
	{
		if (!bIncludeNonPlayerSkills && !Row.AbilityTags.HasTag(PlayerSkillTag))
		{
			continue;
		}
		if (Skills->GetSkillObjectBySkillID(Row.SkillID) && Skills->GetAbilitySpecBySkillID(Row.SkillID))
		{
			++AlreadyOwnedCount;
			continue;
		}
		// Bypass only this registration's capacity check; normal rewards keep their limit.
		if (DiaCharacter->ApplySkillByID(Row.SkillID, true))
		{
			++GrantedCount;
		}
		else
		{
			++FailedCount;
			ReportSkillCheat(this, FString::Printf(TEXT("지급 실패: %d | %s"), Row.SkillID, *Row.SkillName.ToString()));
		}
	}
	ReportSkillCheat(this, FString::Printf(TEXT("%s 스킬 지급: 신규/복구 %d, 기존 보유 %d, 실패 %d. 현재 보유 %d개."),
		bIncludeNonPlayerSkills ? TEXT("전체") : TEXT("플레이어"), GrantedCount, AlreadyOwnedCount, FailedCount,
		Skills->GetSkillIDMapping().Num()));
	ReportSkillCheat(this, TEXT("퀵슬롯 변경: CheatGiveSkill 스킬ID 슬롯번호(1~5). Variant는 별도로 획득/적용합니다."));
#endif
}

void ADiaController::CheatListSkills()
{
#if UE_BUILD_SHIPPING
	return;
#else
	UGASSkillManager* Data = GetGameInstance() ? GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
	if (!IsValid(Data))
	{
		ReportSkillCheat(this, TEXT("스킬 데이터가 없습니다. 플레이를 시작한 뒤 실행하세요."));
		return;
	}
	const ADiaBaseCharacter* DiaCharacter = Cast<ADiaBaseCharacter>(GetPawn());
	const UDiaSkillManagerComponent* Skills = IsValid(DiaCharacter) ? DiaCharacter->GetSkillManagerComponent() : nullptr;
	TArray<FGASSkillData> Rows = Data->GetAllSkillData();
	Rows.Sort([](const FGASSkillData& A, const FGASSkillData& B) { return A.SkillID < B.SkillID; });
	ReportSkillCheat(this, FString::Printf(TEXT("로드된 스킬 %d개 | CheatGiveSkill 스킬ID [슬롯 1~5]"), Rows.Num()));
	for (const FGASSkillData& Row : Rows)
	{
		const bool bOwned = IsValid(Skills) && Skills->GetSkillObjectBySkillID(Row.SkillID) != nullptr;
		const int32 Slot = IsValid(Skills) ? Skills->GetIndexOfSkillID(Row.SkillID) : INDEX_NONE;
		const FString State = Slot != INDEX_NONE ? FString::Printf(TEXT("퀵슬롯 %d"), Slot + 1)
			: (bOwned ? TEXT("보유") : TEXT("미보유"));
		TArray<FString> VariantIDs;
		for (const int32 VariantID : Row.VariantIDs)
		{
			VariantIDs.Add(FString::FromInt(VariantID));
		}
		const FString Variants = VariantIDs.IsEmpty() ? TEXT("없음") : FString::Join(VariantIDs, TEXT(", "));
		ReportSkillCheat(this, FString::Printf(TEXT("%d | %s | %s | Variant: %s"), Row.SkillID, *Row.SkillName.ToString(), *State, *Variants));
	}
#endif
}
