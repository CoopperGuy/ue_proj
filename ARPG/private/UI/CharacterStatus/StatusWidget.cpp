// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/CharacterStatus/StatusWidget.h"
#include "UI/CharacterStatus/StatusSet.h"

#include "Components/ListView.h"
#include "Components/TextBlock.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/DiaAttributeSet.h"

void UStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("UStatusWidget::NativeConstruct called"));
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
				UE_LOG(LogTemp, Warning, TEXT("Checking for Max Attribute: %s, FindPos: %d"), *PropertyName, FindPos);
				if(FindPos != INDEX_NONE)
				{
					//Max Text 제거
					PropertyName.ReplaceInline(TEXT("Max"), TEXT(""));
					FString BaseTagName = FString::Printf(TEXT("AttributeSet.%s"), *PropertyName);
					UE_LOG(LogTemp, Warning, TEXT("Max Attribute Found: %s"), *BaseTagName);
					if (UStatusItemObject* ItemObj = StatusSetMap.FindRef(BaseTagName))
					{
						ItemObj->StatusMaxValue = FString::FromInt(
						FMath::RoundToInt(CachedASC->GetNumericAttribute(Attribute)));
						UE_LOG(LogTemp, Warning, TEXT("Set Max Value for %s to %s"), *BaseTagName, *ItemObj->StatusMaxValue);
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
	int32 FindPos = AttributeName.Find(TEXT("Max"));
	if (FindPos != INDEX_NONE)
	{
		AttributeName.ReplaceInline(TEXT("Max"), TEXT(""));
	}
	AttributeName = FString::Printf(TEXT("AttributeSet.%s"), *AttributeName);
	UStatusItemObject* FindStatusSet = StatusSetMap.FindRef(AttributeName);
	if (IsValid(FindStatusSet))
	{
		FindStatusSet->StatusValue = FString::FromInt(FMath::RoundToInt(Data.NewValue));
		if (FindStatusSet->bShowMaxValue)
		{
			//최대값도 업데이트
			//HACK (임시로 그냥 박아놓음) AttributeSet에서 Tag로 가져오는 함수 작성해야함
			FGameplayAttribute MaxAttribute;
			if (AttributeName == "AttributeSet.Health")
			{
				MaxAttribute = UDiaAttributeSet::GetMaxHealthAttribute();
			}
			else if (AttributeName == "AttributeSet.Mana")
			{
				MaxAttribute = UDiaAttributeSet::GetMaxManaAttribute();
			}

			if (MaxAttribute.IsValid())
			{
				float MaxValue = CachedASC->GetNumericAttribute(MaxAttribute);
				FindStatusSet->StatusMaxValue = FString::FromInt(FMath::RoundToInt(MaxValue));
			}
		}
		StatusList->RequestRefresh();
	}
}
