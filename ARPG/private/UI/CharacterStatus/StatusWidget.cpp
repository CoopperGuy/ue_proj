// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/CharacterStatus/StatusWidget.h"
#include "UI/CharacterStatus/StatusSet.h"

#include "Components/ListView.h"
#include "Components/ListViewBase.h"
#include "Components/TextBlock.h"
#include "Blueprint/IUserObjectListEntry.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Controller/DiaController.h"
#include "GAS/DiaAttributeSet.h"
#include "Logging/ARPGLogChannels.h"

void UStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogARPG_UI, Display, TEXT("UStatusWidget::NativeConstruct called"));
	BindPlayerASCReadyEvent();
	TryInitializeFromPlayer();
}

bool UStatusWidget::TryInitializeFromPlayer()
{
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC))
	{
		UnbindASC();
		ClearStatusSetMap();
		return false;
	}

	APawn* Pawn = PC->GetPawn();
	if (!IsValid(Pawn))
	{
		UnbindASC();
		ClearStatusSetMap();
		return false;
	}

	UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!IsValid(AbilitySystem))
	{
		UnbindASC();
		ClearStatusSetMap();
		return false;
	}

	BindASC(AbilitySystem);
	RebuildStatusList();
	return true;
}

void UStatusWidget::NativeDestruct()
{
	UnbindPlayerASCReadyEvent();
	UnbindASC();
	ClearStatusSetMap();

	Super::NativeDestruct();
}

void UStatusWidget::BindPlayerASCReadyEvent()
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

void UStatusWidget::UnbindPlayerASCReadyEvent()
{
	if (ADiaController* PlayerController = BoundPlayerController.Get(); PlayerController && PlayerASCReadyDelegateHandle.IsValid())
	{
		PlayerController->GetOnPlayerASCReady().Remove(PlayerASCReadyDelegateHandle);
	}

	PlayerASCReadyDelegateHandle.Reset();
	BoundPlayerController.Reset();
}

void UStatusWidget::HandlePlayerASCReady(ADiaBaseCharacter* /*PlayerCharacter*/, UAbilitySystemComponent* /*AbilitySystem*/)
{
	TryInitializeFromPlayer();
}

void UStatusWidget::BindASC(UAbilitySystemComponent* InASC)
{
	if (!IsValid(InASC))
	{
		UnbindASC();
		return;
	}

	if (CachedASC.Get() == InASC && AttributeDelegateHandles.Num() > 0)
	{
		return;
	}

	UnbindASC();
	CachedASC = InASC;

	for (TFieldIterator<FStructProperty> It(UDiaAttributeSet::StaticClass()); It; ++It)
	{
		FStructProperty* Property = *It;
		if (Property && Property->Struct == FGameplayAttributeData::StaticStruct())
		{
			const FGameplayAttribute Attribute(Property);
			const FDelegateHandle Handle = InASC->GetGameplayAttributeValueChangeDelegate(Attribute)
				.AddUObject(this, &ThisClass::OnUpdateStats);
			AttributeDelegateHandles.Add(Attribute, Handle);
		}
	}
}

void UStatusWidget::UnbindASC()
{
	if (UAbilitySystemComponent* AbilitySystem = CachedASC.Get())
	{
		for (const TPair<FGameplayAttribute, FDelegateHandle>& Binding : AttributeDelegateHandles)
		{
			AbilitySystem->GetGameplayAttributeValueChangeDelegate(Binding.Key).Remove(Binding.Value);
		}
	}

	AttributeDelegateHandles.Reset();
	CachedASC.Reset();
}

void UStatusWidget::RebuildStatusList()
{
	ClearStatusSetMap();

	UAbilitySystemComponent* AbilitySystem = CachedASC.Get();
	if (!IsValid(AbilitySystem) || !IsValid(StatusList))
	{
		return;
	}

	for (TFieldIterator<FStructProperty> It(UDiaAttributeSet::StaticClass()); It; ++It)
	{
		FStructProperty* Property = *It;
		if (Property && Property->Struct == FGameplayAttributeData::StaticStruct())
		{
			FString PropertyName = Property->GetName();
			const FString TagName = FString::Printf(TEXT("AttributeSet.%s"), *PropertyName);
			const FGameplayTag AttributeTag = FGameplayTag::RequestGameplayTag(FName(*TagName));
			const FGameplayAttribute Attribute(Property);

			if (IsVisibleStatusTags.Find(AttributeTag))
			{
				UStatusItemObject* NewStatusItemObject = NewObject<UStatusItemObject>(this);
				if (HasMaxStatusTags.Find(AttributeTag))
				{
					NewStatusItemObject->bShowMaxValue = true;
				}
				else
				{
					NewStatusItemObject->bShowMaxValue = false;
				}

				if (IsValid(NewStatusItemObject))
				{
					NewStatusItemObject->StatusName = PropertyName;
					const float InitialValue = AbilitySystem->GetNumericAttribute(Attribute);
					NewStatusItemObject->StatusValue = FString::FromInt(FMath::RoundToInt(InitialValue));
					StatusList->AddItem(NewStatusItemObject);
					StatusSetMap.Add(TagName, NewStatusItemObject);
				}
			}
			else
			{
				int32 FindPos = PropertyName.Find(TEXT("Max"));
				UE_LOG(LogARPG_UI, Verbose, TEXT("Checking for Max Attribute: %s, FindPos: %d"), *PropertyName, FindPos);
				if(FindPos != INDEX_NONE)
				{
					//Max Text 제거
					PropertyName.ReplaceInline(TEXT("Max"), TEXT(""));
					FString BaseTagName = FString::Printf(TEXT("AttributeSet.%s"), *PropertyName);
					UE_LOG(LogARPG_UI, Verbose, TEXT("Max Attribute Found: %s"), *BaseTagName);
					if (UStatusItemObject* ItemObj = StatusSetMap.FindRef(BaseTagName))
					{
						ItemObj->StatusMaxValue = FString::FromInt(
							FMath::RoundToInt(AbilitySystem->GetNumericAttribute(Attribute)));
						UE_LOG(LogARPG_UI, Verbose, TEXT("Set Max Value for %s to %s"), *BaseTagName, *ItemObj->StatusMaxValue);
					}
				}
			}
		}
	}

	StatusList->RequestRefresh();
}

void UStatusWidget::OnUpdateStats(const FOnAttributeChangeData& Data)
{
	UAbilitySystemComponent* AbilitySystem = CachedASC.Get();
	if (!IsValid(AbilitySystem)) return;

	FString AttributeName = Data.Attribute.GetName();
	const bool bIsMaxAttribute = (AttributeName.Find(TEXT("Max")) != INDEX_NONE);
	if (bIsMaxAttribute)
	{
		AttributeName.ReplaceInline(TEXT("Max"), TEXT(""));
	}
	AttributeName = FString::Printf(TEXT("AttributeSet.%s"), *AttributeName);
	UStatusItemObject* FindStatusSet = StatusSetMap.FindRef(AttributeName);

	UE_LOG(LogARPG_UI, Display, TEXT("%s : Old %f , New %f"), *AttributeName, Data.OldValue, Data.NewValue);

	if (!IsValid(FindStatusSet)) return;

	// MaxHealth/MaxMana 변경 시에는 StatusValue(현재값)를 건드리지 않음. Data.NewValue는 최대값이므로.
	if (!bIsMaxAttribute)
	{
		FindStatusSet->StatusValue = FString::FromInt(FMath::RoundToInt(Data.NewValue));
	}

	if (FindStatusSet->bShowMaxValue)
	{
		if (bIsMaxAttribute)
		{
			FindStatusSet->StatusMaxValue = FString::FromInt(FMath::RoundToInt(Data.NewValue));
		}
		else
		{
			//HACK (임시) AttributeSet에서 Tag로 가져오는 함수 작성하면 제거
			FGameplayAttribute MaxAttribute;
			if (AttributeName == TEXT("AttributeSet.Health"))
			{
				MaxAttribute = UDiaAttributeSet::GetMaxHealthAttribute();
			}
			else if (AttributeName == TEXT("AttributeSet.Mana"))
			{
				MaxAttribute = UDiaAttributeSet::GetMaxManaAttribute();
			}

			if (MaxAttribute.IsValid())
			{
				const float MaxValue = AbilitySystem->GetNumericAttribute(MaxAttribute);
				FindStatusSet->StatusMaxValue = FString::FromInt(FMath::RoundToInt(MaxValue));
			}
		}
	}
	RefreshEntryForItem(FindStatusSet);
}

void UStatusWidget::RefreshEntryForItem(UStatusItemObject* Item)
{
	if (!IsValid(StatusList) || !IsValid(Item)) return;

	TArray<UUserWidget*> Entries = StatusList->GetDisplayedEntryWidgets();
	for (UUserWidget* Entry : Entries)
	{
		if (!IsValid(Entry)) continue;
		IUserObjectListEntry* ListEntry = Cast<IUserObjectListEntry>(Entry);
		if (ListEntry && ListEntry->GetListItem() == Item)
		{
			if (UStatusSet* StatusSet = Cast<UStatusSet>(Entry))
			{
				StatusSet->UpdateStatusItemObject(Item);
			}
			break;
		}
	}
}

void UStatusWidget::ClearStatusSetMap()
{
	if (IsValid(StatusList))
	{
		StatusList->ClearListItems();
	}

	StatusSetMap.Reset();
}
