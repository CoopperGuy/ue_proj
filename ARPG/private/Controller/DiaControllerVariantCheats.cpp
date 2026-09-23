#include "Controller/DiaController.h"

#include "DiaBaseCharacter.h"
#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "Engine/GameInstance.h"
#include "Logging/ARPGLogChannels.h"
#include "System/GASSkillManager.h"

#if !UE_BUILD_SHIPPING
namespace
{
	void ReportVariantCheat(ADiaController* Controller, const FString& Message)
	{
		UE_LOG(LogARPG, Display, TEXT("[VariantCheat] %s"), *Message);
		Controller->ClientMessage(Message);
	}

	bool ResolveVariantCheatContext(ADiaController* Controller, bool bMutating,
		UDiaSkillManagerComponent*& Skills, UGASSkillManager*& Data)
	{
		ADiaBaseCharacter* DiaCharacter = Cast<ADiaBaseCharacter>(Controller->GetPawn());
		Skills = IsValid(DiaCharacter) ? DiaCharacter->GetSkillManagerComponent() : nullptr;
		Data = Controller->GetGameInstance() ? Controller->GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
		if (!IsValid(Skills) || !IsValid(Data))
		{
			ReportVariantCheat(Controller, TEXT("플레이를 시작하고 스킬 데이터가 로드된 캐릭터를 조종한 상태에서 실행하세요."));
			return false;
		}
		if (bMutating && !DiaCharacter->HasAuthority())
		{
			ReportVariantCheat(Controller, TEXT("Variant 변경은 단독 플레이 또는 서버 플레이어에서 실행하세요."));
			return false;
		}
		return true;
	}

	TArray<int32> GetVariantCheatSkillIDs(const UDiaSkillManagerComponent* Skills, int32 SkillID)
	{
		TArray<int32> IDs;
		if (SkillID > 0)
		{
			IDs.Add(SkillID);
		}
		else
		{
			for (const USkillObject* Skill : Skills->GetSkillIDMapping())
			{
				if (IsValid(Skill)) IDs.AddUnique(Skill->GetSkillID());
			}
			IDs.Sort();
		}
		return IDs;
	}

	struct FVariantCheatCounts
	{
		int32 Granted = 0;
		int32 Applied = 0;
		int32 Failed = 0;
	};

	bool GrantCheatVariant(ADiaController* Controller, UDiaSkillManagerComponent* Skills,
		UGASSkillManager* Data, int32 SkillID, int32 VariantID, bool bApply, FVariantCheatCounts& Counts)
	{
		const FGASSkillData* Row = Data->GetSkillDataPtr(SkillID);
		const USkillObject* Skill = Skills->GetSkillObjectBySkillID(SkillID);
		FString Failure;
		if (!Row)
		{
			Failure = TEXT("로드된 스킬이 없습니다. CheatListSkills로 확인하세요.");
		}
		else if (!IsValid(Skill))
		{
			Failure = FString::Printf(TEXT("미보유 스킬입니다. 먼저 CheatGiveSkill %d 또는 CheatGiveAllSkills를 실행하세요."), SkillID);
		}
		else if (!Row->VariantIDs.Contains(VariantID))
		{
			Failure = TEXT("해당 스킬에 할당되지 않은 variant입니다. CheatListSkillVariants 스킬ID로 확인하세요.");
		}
		else if (!Skill->HasSkillVariantID(VariantID))
		{
			Failure = TEXT("현재 스킬의 캐시와 테이블 할당이 다릅니다. 새 PIE 세션에서 다시 지급하세요.");
		}
		else if (!Data->GetSkllVariantDataPtr(VariantID))
		{
			Failure = TEXT("Variant 데이터가 로드되지 않았습니다.");
		}
		if (!Failure.IsEmpty())
		{
			++Counts.Failed;
			ReportVariantCheat(Controller, FString::Printf(TEXT("스킬 %d / variant %d: %s"), SkillID, VariantID, *Failure));
			return false;
		}

		// Unlock returns false for duplicates; already-owned variants can still be applied.
		if (!Skill->HasOwnedVariantID(VariantID))
		{
			if (!Skills->TryUnlockSkillVariantByID(SkillID, VariantID))
			{
				++Counts.Failed;
				ReportVariantCheat(Controller, FString::Printf(TEXT("획득 실패: 스킬 %d / variant %d"), SkillID, VariantID));
				return false;
			}
			++Counts.Granted;
		}
		Skill = Skills->GetSkillObjectBySkillID(SkillID);
		if (bApply && (!IsValid(Skill) || !Skill->GetVariantApplyIDs().Contains(VariantID)))
		{
			if (!Skills->TryApplySkillVariantByID(SkillID, VariantID))
			{
				++Counts.Failed;
				ReportVariantCheat(Controller, FString::Printf(TEXT("적용 실패: 스킬 %d / variant %d"), SkillID, VariantID));
				return false;
			}
			++Counts.Applied;
		}
		return true;
	}
}
#endif

void ADiaController::CheatGiveSkillVariant(int32 SkillID, int32 VariantID, bool bApply)
{
#if UE_BUILD_SHIPPING
	return;
#else
	if (SkillID <= 0 || VariantID <= 0)
	{
		ReportVariantCheat(this, TEXT("사용법: CheatGiveSkillVariant 스킬ID variantID [즉시적용 true/false]. 기본값 true."));
		return;
	}
	UDiaSkillManagerComponent* Skills = nullptr;
	UGASSkillManager* Data = nullptr;
	if (!ResolveVariantCheatContext(this, true, Skills, Data)) return;
	FVariantCheatCounts Counts;
	if (GrantCheatVariant(this, Skills, Data, SkillID, VariantID, bApply, Counts))
	{
		const USkillObject* Skill = Skills->GetSkillObjectBySkillID(SkillID);
		const FSkillVariantData* Variant = Data->GetSkllVariantDataPtr(VariantID);
		ReportVariantCheat(this, FString::Printf(TEXT("스킬 %d | %d %s | 보유, %s | 신규 획득 %d / 신규 적용 %d"),
			SkillID, VariantID, *Variant->VariantName.ToString(),
			IsValid(Skill) && Skill->GetVariantApplyIDs().Contains(VariantID) ? TEXT("적용 중") : TEXT("미적용"), Counts.Granted, Counts.Applied));
	}
#endif
}

void ADiaController::CheatGiveAllSkillVariants(int32 SkillID, bool bApply)
{
#if UE_BUILD_SHIPPING
	return;
#else
	if (SkillID < 0)
	{
		ReportVariantCheat(this, TEXT("사용법: CheatGiveAllSkillVariants [스킬ID 또는 0] [즉시적용 true/false]. 0은 모든 보유 스킬."));
		return;
	}
	UDiaSkillManagerComponent* Skills = nullptr;
	UGASSkillManager* Data = nullptr;
	if (!ResolveVariantCheatContext(this, true, Skills, Data)) return;
	const TArray<int32> SkillIDs = GetVariantCheatSkillIDs(Skills, SkillID);
	if (SkillIDs.IsEmpty())
	{
		ReportVariantCheat(this, TEXT("보유 스킬이 없습니다. 먼저 CheatGiveAllSkills를 실행하세요."));
		return;
	}
	FVariantCheatCounts Counts;
	int32 PairCount = 0;
	for (const int32 ID : SkillIDs)
	{
		const FGASSkillData* Row = Data->GetSkillDataPtr(ID);
		if (!Row || !IsValid(Skills->GetSkillObjectBySkillID(ID)))
		{
			++Counts.Failed;
			ReportVariantCheat(this, FString::Printf(TEXT("스킬 %d의 데이터 또는 보유 정보가 없습니다. CheatGiveSkill %d로 먼저 지급하세요."), ID, ID));
			continue;
		}
		TArray<int32> VariantIDs;
		for (const int32 VariantID : Row->VariantIDs) VariantIDs.AddUnique(VariantID);
		VariantIDs.Sort();
		for (const int32 VariantID : VariantIDs)
		{
			++PairCount;
			GrantCheatVariant(this, Skills, Data, ID, VariantID, bApply, Counts);
		}
	}
	ReportVariantCheat(this, FString::Printf(TEXT("Variant 일괄 %s: 스킬 %d개 / 할당 %d건 | 신규 획득 %d, 신규 적용 %d, 실패 %d."),
		bApply ? TEXT("획득·적용") : TEXT("획득"), SkillIDs.Num(), PairCount, Counts.Granted, Counts.Applied, Counts.Failed));
	ReportVariantCheat(this, TEXT("기존 보유·적용 상태는 유지합니다. 확인: CheatListSkillVariants [스킬ID]. 적용 해제: CheatClearSkillVariants [스킬ID]."));
#endif
}

void ADiaController::CheatClearSkillVariants(int32 SkillID)
{
#if UE_BUILD_SHIPPING
	return;
#else
	if (SkillID < 0)
	{
		ReportVariantCheat(this, TEXT("사용법: CheatClearSkillVariants [스킬ID 또는 0]. 0은 모든 보유 스킬. 획득한 variant는 보존합니다."));
		return;
	}
	UDiaSkillManagerComponent* Skills = nullptr;
	UGASSkillManager* Data = nullptr;
	if (!ResolveVariantCheatContext(this, true, Skills, Data)) return;
	int32 ClearedCount = 0;
	int32 SkillCount = 0;
	for (const int32 ID : GetVariantCheatSkillIDs(Skills, SkillID))
	{
		const int32 Cleared = Skills->ClearAppliedSkillVariantsByID(ID);
		if (Cleared == INDEX_NONE)
		{
			ReportVariantCheat(this, FString::Printf(TEXT("스킬 %d를 보유하고 있지 않습니다."), ID));
			continue;
		}
		++SkillCount;
		ClearedCount += Cleared;
	}
	ReportVariantCheat(this, FString::Printf(TEXT("스킬 %d개의 variant 적용 %d건을 해제했습니다. 획득 상태는 유지하며 다음 시전부터 반영됩니다. 이미 생성된 투사체·장판·강화 효과는 유지됩니다."), SkillCount, ClearedCount));
#endif
}

void ADiaController::CheatListSkillVariants(int32 SkillID)
{
#if UE_BUILD_SHIPPING
	return;
#else
	if (SkillID < 0)
	{
		ReportVariantCheat(this, TEXT("사용법: CheatListSkillVariants [스킬ID 또는 0]. 0은 모든 보유 스킬."));
		return;
	}
	UDiaSkillManagerComponent* Skills = nullptr;
	UGASSkillManager* Data = nullptr;
	if (!ResolveVariantCheatContext(this, false, Skills, Data)) return;
	const TArray<int32> SkillIDs = GetVariantCheatSkillIDs(Skills, SkillID);
	if (SkillIDs.IsEmpty())
	{
		ReportVariantCheat(this, TEXT("보유 스킬이 없습니다. CheatListSkillVariants 스킬ID로 개별 할당 목록을 볼 수 있습니다."));
		return;
	}
	for (const int32 ID : SkillIDs)
	{
		const FGASSkillData* Row = Data->GetSkillDataPtr(ID);
		if (!Row)
		{
			ReportVariantCheat(this, FString::Printf(TEXT("로드된 스킬 ID %d가 없습니다."), ID));
			continue;
		}
		const USkillObject* Skill = Skills->GetSkillObjectBySkillID(ID);
		ReportVariantCheat(this, FString::Printf(TEXT("%d %s | %s | 할당 variant %d개"), ID, *Row->SkillName.ToString(),
			IsValid(Skill) ? TEXT("보유 스킬") : TEXT("미보유 스킬"), Row->VariantIDs.Num()));
		const TSet<int32> Applied = IsValid(Skill) ? Skill->GetVariantApplyIDs() : TSet<int32>();
		TArray<int32> VariantIDs = Row->VariantIDs;
		VariantIDs.Sort();
		for (const int32 VariantID : VariantIDs)
		{
			const FSkillVariantData* Variant = Data->GetSkllVariantDataPtr(VariantID);
			if (!Variant)
			{
				ReportVariantCheat(this, FString::Printf(TEXT("  %d | 데이터 없음"), VariantID));
				continue;
			}
			const TCHAR* State = Applied.Contains(VariantID) ? TEXT("적용 중")
				: (IsValid(Skill) && Skill->HasOwnedVariantID(VariantID) ? TEXT("보유·미적용") : TEXT("미보유"));
			ReportVariantCheat(this, FString::Printf(TEXT("  %d | %s | %s | %s"), VariantID,
				*Variant->VariantName.ToString(), State, *Variant->Description.ToString()));
		}
	}
#endif
}
