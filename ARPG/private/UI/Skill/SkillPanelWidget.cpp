// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skill/SkillPanelWidget.h"
#include "Components/ScrollBox.h"
#include "UI/Skill/SkillSlotWidget.h"

#include "DiaInstance.h"
#include "Skill/DiaSkillManager.h"

void USkillPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

//SkillPanel 초기화
//스킬 데이터를 가져와서 초기화 한다.
//캐릭터에 맞는 스킬을 가져온다.

//skillmanager에서 스킬 데이터를 가져와서
//패널에 등록한다.
void USkillPanelWidget::InitializeSkillPanel()
{
	UDiaInstance* DiaInstance = (GetWorld()->GetGameInstance<UDiaInstance>());
	if (!IsValid(DiaInstance))
		return;

	UDiaSkillManager* SkillManager = DiaInstance->GetSkillManager();
	if (!IsValid(SkillManager))
		return;

	//기본 스킬 데이터 가져오기
	//여기서 스킬 레벨 가져오는게 좀 더 좋긴한데 일단은 현상태
	for (const auto& SkillPair : SkillManager->GetSkillDataMap())
	{
		const FSkillData& SkillData = SkillPair.Value;
		AddSkillToPanel(SkillData, true);
	}

}

void USkillPanelWidget::ToggleSkillPanel()
{
}

void USkillPanelWidget::RegisterSkillPanel(int32 SkillID)
{
}

void USkillPanelWidget::AddSkillToPanel(const FSkillData& SkillData, int32 SkillID, bool bIsActiveSkill)
{
	FDiaSkillBaseInfo SkillInfo = FDiaSkillBaseInfo(SkillData, SkillID, 1);
}


