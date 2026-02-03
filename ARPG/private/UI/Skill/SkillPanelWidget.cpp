// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skill/SkillPanelWidget.h"
#include "Components/ScrollBox.h"
#include "CommonListView.h"

#include "UI/Skill/SkillSlotWidget.h"

#include "DiaComponent/Skill/SkillObject.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"

#include "Character/DiaCharacter.h"

#include "DiaInstance.h"
#include "Skill/DiaSkillManager.h"

void USkillPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogTemp, Log, TEXT("USkillPanelWidget::NativeConstruct - SkillPanelWidget constructed"));

	InitializeSkillPanel();

	ActiveSkillListView->OnItemClicked().AddUObject(this, &USkillPanelWidget::HandleItemClicked);
}

//이거 누르면 sublistview에 variants 업데이트 하기
void USkillPanelWidget::HandleItemClicked(UObject* Item)
{
	USkillInfoObject* ClickedSkillInfo = Cast<USkillInfoObject>(Item);
	if (!ClickedSkillInfo)
		return;

	// 여기에 SubSkillListView 업데이트 로직 추가
	// 예: SubSkillListView->ClearListItems(); 그런 다음 새로운 항목 추가
	
	APawn* Pawn = GetOwningPlayerPawn();
	ADiaCharacter* Character = Cast<ADiaCharacter>(Pawn);
	if (!Character)
		return;

	TArray<UDiaSkillVariant*> SkillVariants;
	Character->GetSkillVariantsFromSkillID(ClickedSkillInfo->SkillID, SkillVariants);

	SubSkillListView->ClearListItems();

	for(const auto & Variant : SkillVariants)
	{
		if (!Variant)
			continue;
		USkillInfoObject* NewVariantInfo = NewObject<USkillInfoObject>(this);
		NewVariantInfo->SkillID = Variant->GetSkillID();
		NewVariantInfo->SkillName = Variant->GetSkillVariantName();
		NewVariantInfo->SkillLevel = 1;
		NewVariantInfo->SkillIcon = nullptr; 

		// 아이콘 설정 등 추가 정보 설정 가능
		SubSkillListView->AddItem(NewVariantInfo);
	}

}

void USkillPanelWidget::InitializeSkillPanel()
{
	UDiaInstance* DiaInstance = (GetWorld()->GetGameInstance<UDiaInstance>());
	if (!IsValid(DiaInstance))
	{
		UE_LOG(LogTemp, Warning, TEXT("USkillPanelWidget::InitializeSkillPanel - DiaInstance is invalid"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("USkillPanelWidget::InitializeSkillPanel - DiaInstance is valid"));
}

void USkillPanelWidget::ToggleSkillPanel()
{
	if (IsVisible())
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		SetVisibility(ESlateVisibility::Visible);
	}
}

void USkillPanelWidget::AddSkillToPanel(const FGASSkillData& SkillData, int32 SkillID, bool bIsActiveSkill)
{
	if (!IsValid(SkillSlotWidgetClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("USkillPanelWidget::AddSkillToPanel - SkillSlotWidgetClass is invalid, SkillID: %d"), SkillID);
		return;
	}

	UTexture2D* IconTexture = nullptr;
	if (SkillData.Icon.IsValid())
	{
		IconTexture = SkillData.Icon.LoadSynchronous();
		UE_LOG(LogTemp, Log, TEXT("USkillPanelWidget::AddSkillToPanel - Icon loaded for SkillID: %d, SkillName: %s"),
			SkillID, *SkillData.SkillName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USkillPanelWidget::AddSkillToPanel - Icon is invalid for SkillID: %d, SkillName: %s"),
			SkillID, *SkillData.SkillName.ToString());
	}

	USkillInfoObject* NewSkillInfoObject = NewObject<USkillInfoObject>(this);
	if (!IsValid(NewSkillInfoObject))
	{
		UE_LOG(LogTemp, Warning, TEXT("USkillPanelWidget::AddSkillToPanel - Failed to create USkillInfoObject for SkillID: %d"), SkillID);
		return;
	}

	NewSkillInfoObject->SkillID = SkillID;
	NewSkillInfoObject->SkillIcon = IconTexture;
	NewSkillInfoObject->SkillName = SkillData.SkillName;
	NewSkillInfoObject->SkillLevel = 1;

	// 3) 어느 리스트에 넣을지만 분기
	if (bIsActiveSkill && IsValid(ActiveSkillListView))
	{
		ActiveSkillListView->AddItem(NewSkillInfoObject);
	}
	else if (!bIsActiveSkill && IsValid(SubSkillListView))
	{
		SubSkillListView->AddItem(NewSkillInfoObject);
	}

}

void USkillPanelWidget::RegisterSkillList(const TArray<USkillObject*>& Skills)
{
	int32 Index = 0;
	for (USkillObject* SkillObject : Skills)
	{
		if (!IsValid(SkillObject))
		{
			UE_LOG(LogTemp, Warning, TEXT("USkillPanelWidget::RegisterSkillList - SkillObject[%d] is invalid"), Index);
			++Index;
			continue;
		}

		const FGASSkillData* SkillData = SkillObject->GetSkillData();
		const int32 SkillID = SkillObject->GetSkillID();

		if (!SkillData)
		{
			UE_LOG(LogTemp, Warning, TEXT("USkillPanelWidget::RegisterSkillList - SkillData is null for SkillObject[%d], SkillID: %d"),
				Index, SkillID);
			++Index;
			continue;
		}


		AddSkillToPanel(*SkillData, SkillID, true);
		++Index;

	}
}


