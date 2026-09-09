// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Skill/SkillPanelWidget.h"
#include "CommonListView.h"

#include "UI/Skill/SkillSlotWidget.h"

#include "DiaComponent/DiaSkillManagerComponent.h"
#include "DiaComponent/Skill/SkillObject.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"

#include "Character/DiaCharacter.h"
#include "Controller/DiaController.h"

#include "System/GASSkillManager.h"
#include "Logging/ARPGLogChannels.h"

void USkillPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogARPG, Log, TEXT("USkillPanelWidget::NativeConstruct - SkillPanelWidget constructed"));

	BindPlayerASCReadyEvent();
	TryInitializeFromPlayer();

	if (IsValid(ActiveSkillListView))
	{
		ActiveSkillListView->OnItemClicked().RemoveAll(this);
		ActiveSkillListView->OnItemClicked().AddUObject(this, &USkillPanelWidget::HandleItemClicked);
	}
}

void USkillPanelWidget::NativeDestruct()
{
	UnbindPlayerASCReadyEvent();
	UnbindSkillManagerDelegates();

	if (IsValid(ActiveSkillListView))
	{
		ActiveSkillListView->OnItemClicked().RemoveAll(this);
	}

	Super::NativeDestruct();
}

UDiaSkillManagerComponent* USkillPanelWidget::GetSkillManagerComponent() const
{
	const APawn* OwningPawn = GetOwningPlayerPawn();
	return IsValid(OwningPawn) ? OwningPawn->FindComponentByClass<UDiaSkillManagerComponent>() : nullptr;
}

bool USkillPanelWidget::TryInitializeFromPlayer()
{
	const bool bHasRequiredWidgets = IsValid(ActiveSkillListView) && IsValid(SubSkillListView);
	const bool bHasSkillManager = IsValid(GetSkillManagerComponent());

	InitializeSkillPanel();
	BindSkillManagerDelegates();

	return bHasRequiredWidgets && bHasSkillManager;
}

void USkillPanelWidget::BindPlayerASCReadyEvent()
{
	ADiaController* PlayerController = Cast<ADiaController>(GetOwningPlayer());
	if (!IsValid(PlayerController))
	{
		return;
	}

	if (BoundPlayerController.Get() == PlayerController && PlayerASCReadyDelegateHandle.IsValid())
	{
		return;
	}

	UnbindPlayerASCReadyEvent();
	BoundPlayerController = PlayerController;
	PlayerASCReadyDelegateHandle = PlayerController->GetOnPlayerASCReady()
		.AddUObject(this, &ThisClass::HandlePlayerASCReady);
}

void USkillPanelWidget::UnbindPlayerASCReadyEvent()
{
	if (ADiaController* PlayerController = BoundPlayerController.Get(); PlayerController && PlayerASCReadyDelegateHandle.IsValid())
	{
		PlayerController->GetOnPlayerASCReady().Remove(PlayerASCReadyDelegateHandle);
	}

	PlayerASCReadyDelegateHandle.Reset();
	BoundPlayerController.Reset();
}

void USkillPanelWidget::HandlePlayerASCReady(ADiaBaseCharacter* /*PlayerCharacter*/, UAbilitySystemComponent* /*AbilitySystem*/)
{
	TryInitializeFromPlayer();
}

void USkillPanelWidget::BindSkillManagerDelegates()
{
	UDiaSkillManagerComponent* SkillManager = GetSkillManagerComponent();
	if (BoundSkillManager.Get() == SkillManager &&
		SkillRegisteredHandle.IsValid() &&
		SkillLevelChangedHandle.IsValid() &&
		SkillVariantAddedHandle.IsValid())
	{
		return;
	}

	UnbindSkillManagerDelegates();

	if (!IsValid(SkillManager))
	{
		return;
	}

	BoundSkillManager = SkillManager;

	SkillRegisteredHandle = SkillManager->OnSkillRegistered.AddUObject(this, &ThisClass::HandleSkillRegistered);
	SkillLevelChangedHandle = SkillManager->OnSkillLevelChanged.AddUObject(this, &ThisClass::HandleSkillLevelChanged);
	SkillVariantAddedHandle = SkillManager->OnSkillVariantAdded.AddUObject(this, &ThisClass::HandleSkillVariantAdded);
}

void USkillPanelWidget::UnbindSkillManagerDelegates()
{
	if (UDiaSkillManagerComponent* SkillManager = BoundSkillManager.Get())
	{
		SkillManager->OnSkillRegistered.Remove(SkillRegisteredHandle);
		SkillManager->OnSkillLevelChanged.Remove(SkillLevelChangedHandle);
		SkillManager->OnSkillVariantAdded.Remove(SkillVariantAddedHandle);
	}

	SkillRegisteredHandle.Reset();
	SkillLevelChangedHandle.Reset();
	SkillVariantAddedHandle.Reset();
	BoundSkillManager.Reset();
}

//이거 누르면 sublistview에 variants 업데이트 하기
void USkillPanelWidget::HandleItemClicked(UObject* Item)
{
	USkillInfoObject* ClickedSkillInfo = Cast<USkillInfoObject>(Item);
	if (!ClickedSkillInfo)
		return;

	// 여기에 SubSkillListView 업데이트 로직 추가
	// 예: SubSkillListView->ClearListItems(); 그런 다음 새로운 항목 추가
	
	CurrentSelectedSkillID = ClickedSkillInfo->SkillID;
	RefreshOwnedSkillVariants(CurrentSelectedSkillID);
		// 아이콘 설정 등 추가 정보 설정 가능

}

void USkillPanelWidget::RegisterSkillVariant(int32 SkillID, int32 VariantID)
{
	if (VariantID <= 0)
	{
		return;
	}

	if (CurrentSelectedSkillID != SkillID)
	{
		return;
	}

	RefreshOwnedSkillVariants(SkillID);
}

void USkillPanelWidget::RefreshOwnedSkillVariants(int32 SkillID)
{
	APawn* Pawn = GetOwningPlayerPawn();
	ADiaCharacter* Character = Cast<ADiaCharacter>(Pawn);
	if (!Character || !IsValid(SubSkillListView))
	{
		return;
	}

	TArray<UDiaSkillVariant*> SkillVariants;
	Character->GetOwnedSkillVariantsFromSkillID(SkillID, SkillVariants);
	UGASSkillManager* GASSkillManager = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UGASSkillManager>()
		: nullptr;

	SubSkillListView->ClearListItems();

	for (const auto& Variant : SkillVariants)
	{
		if (!Variant)
		{
			continue;
		}

		USkillInfoObject* NewVariantInfo = NewObject<USkillInfoObject>(this);
		NewVariantInfo->SkillID = Variant->GetSkillID();
		NewVariantInfo->SkillName = Variant->GetSkillVariantName();
		NewVariantInfo->SkillLevel = 1;
		const FSkillVariantData* VariantData = GASSkillManager
			? GASSkillManager->GetSkllVariantDataPtr(Variant->GetSkillID())
			: nullptr;
		NewVariantInfo->SkillIcon = VariantData ? VariantData->Icon.LoadSynchronous() : nullptr;
		NewVariantInfo->isMainSkill = false;
		NewVariantInfo->MainSkillID = SkillID;
		SubSkillListView->AddItem(NewVariantInfo);
	}
}

void USkillPanelWidget::InitializeSkillPanel()
{
	if (!IsValid(ActiveSkillListView) || !IsValid(SubSkillListView))
	{
		UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::InitializeSkillPanel - Required list view is invalid"));
		return;
	}

	ActiveSkillListView->ClearListItems();
	SubSkillListView->ClearListItems();
	RegisteredSkillInfoMap.Reset();
	CurrentSelectedSkillID = INDEX_NONE;

	UDiaSkillManagerComponent* SkillManager = GetSkillManagerComponent();
	if (!IsValid(SkillManager))
	{
		UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::InitializeSkillPanel - SkillManagerComponent is invalid"));
		return;
	}

	RegisterSkillList(SkillManager->GetSkillIDMapping());
}

void USkillPanelWidget::HandleSkillRegistered(int32 SkillID, int32 /*SlotIndex*/)
{
	UDiaSkillManagerComponent* SkillManager = GetSkillManagerComponent();
	if (!IsValid(SkillManager))
	{
		return;
	}

	RegisterSkill(SkillManager->GetSkillObjectBySkillID(SkillID));
}

void USkillPanelWidget::HandleSkillLevelChanged(int32 SkillID, int32 NewLevel)
{
	UpdateSkillLevel(SkillID, NewLevel);
}

void USkillPanelWidget::HandleSkillVariantAdded(int32 SkillID, int32 VariantID)
{
	RegisterSkillVariant(SkillID, VariantID);
}

USkillInfoObject* USkillPanelWidget::FindActiveSkillInfoObject(int32 SkillID) const
{
	if (const TWeakObjectPtr<USkillInfoObject>* SkillInfoObject = RegisteredSkillInfoMap.Find(SkillID))
	{
		return SkillInfoObject->Get();
	}

	return nullptr;
}

void USkillPanelWidget::AddSkillToPanel(const FGASSkillData& SkillData, int32 SkillID, int32 SkillLevel, bool bIsActiveSkill)
{
	if (!IsValid(SkillSlotWidgetClass))
	{
		UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::AddSkillToPanel - SkillSlotWidgetClass is invalid, SkillID: %d"), SkillID);
		return;
	}

	if (FindActiveSkillInfoObject(SkillID))
	{
		UE_LOG(LogARPG, Verbose, TEXT("USkillPanelWidget::AddSkillToPanel - SkillID %d is already registered"), SkillID);
		return;
	}

	UTexture2D* IconTexture = nullptr;
	if (SkillData.Icon.IsValid())
	{
		IconTexture = SkillData.Icon.LoadSynchronous();
		UE_LOG(LogARPG, Log, TEXT("USkillPanelWidget::AddSkillToPanel - Icon loaded for SkillID: %d, SkillName: %s"),
			SkillID, *SkillData.SkillName.ToString());
	}
	else
	{
		UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::AddSkillToPanel - Icon is invalid for SkillID: %d, SkillName: %s"),
			SkillID, *SkillData.SkillName.ToString());
	}

	USkillInfoObject* NewSkillInfoObject = NewObject<USkillInfoObject>(this);
	if (!IsValid(NewSkillInfoObject))
	{
		UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::AddSkillToPanel - Failed to create USkillInfoObject for SkillID: %d"), SkillID);
		return;
	}

	NewSkillInfoObject->SkillID = SkillID;
	NewSkillInfoObject->SkillIcon = IconTexture;
	NewSkillInfoObject->SkillName = SkillData.SkillName;
	NewSkillInfoObject->SkillLevel = SkillLevel;
	NewSkillInfoObject->isMainSkill = bIsActiveSkill;
	// 3) 어느 리스트에 넣을지만 분기
	if (IsValid(ActiveSkillListView))
	{
		ActiveSkillListView->AddItem(NewSkillInfoObject);
		RegisteredSkillInfoMap.Add(SkillID, NewSkillInfoObject);
	}

}

void USkillPanelWidget::RegisterSkill(const USkillObject* Skill)
{
	if (!IsValid(Skill))
	{
		UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::RegisterSkill - SkillObject is invalid"));
		return;
	}

	const FGASSkillData* SkillData = Skill->GetSkillData();
	const int32 SkillID = Skill->GetSkillID();

	if (!SkillData)
	{
		UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::RegisterSkill - SkillData is null, SkillID: %d"), SkillID);
		return;
	}

	AddSkillToPanel(*SkillData, SkillID, Skill->GetSkillLevel(), true);
}

bool USkillPanelWidget::UpdateSkillLevel(int32 SkillID, int32 NewLevel)
{
	USkillInfoObject* SkillInfoObject = FindActiveSkillInfoObject(SkillID);
	if (!IsValid(SkillInfoObject))
	{
		UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::UpdateSkillLevel - SkillInfoObject is null, SkillID: %d"), SkillID);
		return false;
	}

	SkillInfoObject->SkillLevel = NewLevel;

	if (IsValid(ActiveSkillListView))
	{
		TArray<UUserWidget*> Entries = ActiveSkillListView->GetDisplayedEntryWidgets();
		for (UUserWidget* Entry : Entries)
		{
			if (!IsValid(Entry))
			{
				continue;
			}

			IUserObjectListEntry* ListEntry = Cast<IUserObjectListEntry>(Entry);
			if (ListEntry && ListEntry->GetListItem() == SkillInfoObject)
			{
				if (USkillSlotWidget* SkillSlotWidget = Cast<USkillSlotWidget>(Entry))
				{
					SkillSlotWidget->UpdateSkillInfoObject(SkillInfoObject);
				}
				break;
			}
		}
	}

	return true;
}

void USkillPanelWidget::RegisterSkillList(const TArray<USkillObject*>& Skills)
{
	int32 Index = 0;
	for (USkillObject* SkillObject : Skills)
	{
		if (!IsValid(SkillObject))
		{
			UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::RegisterSkillList - SkillObject[%d] is invalid"), Index);
			++Index;
			continue;
		}

		const FGASSkillData* SkillData = SkillObject->GetSkillData();
		const int32 SkillID = SkillObject->GetSkillID();

		if (!SkillData)
		{
			UE_LOG(LogARPG, Warning, TEXT("USkillPanelWidget::RegisterSkillList - SkillData is null for SkillObject[%d], SkillID: %d"),
				Index, SkillID);
			++Index;
			continue;
		}


		AddSkillToPanel(*SkillData, SkillID, SkillObject->GetSkillLevel(), true);
		++Index;

	}
}


