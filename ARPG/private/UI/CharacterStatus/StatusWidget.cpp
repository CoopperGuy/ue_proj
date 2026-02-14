// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/CharacterStatus/StatusWidget.h"
#include "UI/CharacterStatus/StatusSet.h"

#include "Components/ListView.h"
#include "Components/ListViewBase.h"
#include "Components/TextBlock.h"
#include "Blueprint/IUserObjectListEntry.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/DiaAttributeSet.h"

DEFINE_LOG_CATEGORY_STATIC(LogARPG_UI, Log, All);

void UStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogARPG_UI, Display, TEXT("UStatusWidget::NativeConstruct called"));
	// PlayerController와 Pawn 가져오기
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) return;

	APawn* Pawn = PC->GetPawn();
	if (!IsValid(Pawn)) return;

	// AbilitySystemComponent 가져오기 및 캐싱
	CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!IsValid(CachedASC)) return;

	for (TFieldIterator<FStructProperty> It(UDiaAttributeSet::StaticClass()); It; ++It)
	{
		FStructProperty* Property = *It;
		if (Property && Property->Struct == FGameplayAttributeData::StaticStruct())
		{
			FString PropertyName = Property->GetName();
			FString TagName = FString::Printf(TEXT("AttributeSet.%s"), *PropertyName);
			FGameplayTag AttributeTag = FGameplayTag::RequestGameplayTag(FName(*TagName));
			FGameplayAttribute Attribute(Property);
			CachedASC->GetGameplayAttributeValueChangeDelegate(Attribute)
				.AddUObject(this, &UStatusWidget::OnUpdateStats);

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
					float InitialValue = CachedASC->GetNumericAttribute(Attribute);
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
						FMath::RoundToInt(CachedASC->GetNumericAttribute(Attribute)));
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
	if (!IsValid(CachedASC)) return;

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
				float MaxValue = CachedASC->GetNumericAttribute(MaxAttribute);
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
