// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/DiaItemDebugWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/TextBlock.h"
#include "DiaComponent/DiaOptionManagerComponent.h"
#include "DiaComponent/UI/DiaEquipmentComponent.h"
#include "GAS/DiaAttributeSet.h"
#include "System/ItemSubsystem.h"

namespace
{
	FString GetSlotName(EEquipmentSlot SlotType)
	{
		FString FullName = UEnum::GetValueAsString(SlotType);
		FString Prefix;
		FString ShortName;
		if (FullName.Split(TEXT("::"), &Prefix, &ShortName))
		{
			return ShortName;
		}
		return FullName;
	}
}

void UDiaItemDebugWidget::SetDebugSource(UDiaEquipmentComponent* InEquipmentComponent, UDiaOptionManagerComponent* InOptionManagerComponent, UAbilitySystemComponent* InASC)
{
	EquipmentComponent = InEquipmentComponent;
	OptionManagerComponent = InOptionManagerComponent;
	AbilitySystemComponent = InASC;
	RefreshDebugText();
}

void UDiaItemDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshAccumulator += InDeltaTime;
	if (RefreshAccumulator >= 0.25f)
	{
		RefreshAccumulator = 0.0f;
		RefreshDebugText();
	}
}

void UDiaItemDebugWidget::RefreshDebugText()
{
	if (!DebugText)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiaItemDebugWidget: DebugText is not bound"));
		return;
	}

	FString Text;
	Text += TEXT("[Item Debug]\n\n");

	Text += TEXT("== Equipment Slots ==\n");
	if (!IsValid(EquipmentComponent))
	{
		Text += TEXT("EquipmentComponent: MISSING\n\n");
	}
	else
	{
		for (int32 SlotIndex = static_cast<int32>(EEquipmentSlot::EES_Head);
			SlotIndex < static_cast<int32>(EEquipmentSlot::EES_Max);
			++SlotIndex)
		{
			const EEquipmentSlot SlotType = static_cast<EEquipmentSlot>(SlotIndex);
			const FEquippedItem* EquippedItem = EquipmentComponent->GetEquippedItem(SlotType);
			const FString ItemID = EquippedItem
				? EquippedItem->ItemInstance.ItemID.ToString()
				: TEXT("Empty");

			Text += FString::Printf(TEXT("%-8s : %s\n"), *GetSlotName(SlotType), *ItemID);
		}
		Text += TEXT("\n");
	}

	Text += TEXT("== Active Options ==\n");
	if (!IsValid(OptionManagerComponent))
	{
		Text += TEXT("OptionManagerComponent: MISSING\n\n");
	}
	else
	{
		const TMap<FName, FDiaActualItemOption>& ActiveOptions = OptionManagerComponent->GetAllOptions();
		Text += FString::Printf(TEXT("Count: %d\n"), ActiveOptions.Num());
		if (ActiveOptions.Num() == 0)
		{
			Text += TEXT("None\n");
		}
		else
		{
			for (const auto& Pair : ActiveOptions)
			{
				const FDiaActualItemOption& Option = Pair.Value;
				Text += FString::Printf(TEXT("%s | %s | %.2f\n"),
					*Pair.Key.ToString(),
					*Option.GrantedTag.ToString(),
					Option.Value);
			}
		}
		Text += TEXT("\n");
	}

	Text += TEXT("== Attributes ==\n");
	if (!IsValid(AbilitySystemComponent))
	{
		Text += TEXT("AbilitySystemComponent: MISSING\n\n");
	}
	else
	{
		TArray<FGameplayAttribute> OutAttributes;
		AbilitySystemComponent->GetAllAttributes(OutAttributes);
		for (const FGameplayAttribute& Attribute : OutAttributes)
		{
			Text += FString::Printf(TEXT("%-24s : %.2f\n"),
				*Attribute.GetName(),
				AbilitySystemComponent->GetNumericAttribute(Attribute));
		}
		Text += TEXT("\n");
	}

	Text += TEXT("== Equip Verification ==\n");
	if (!IsValid(EquipmentComponent))
	{
		Text += TEXT("No verification source.\n");
	}
	else
	{
		const TArray<FEquipmentDebugEvent>& Events = EquipmentComponent->GetDebugEvents();
		if (Events.Num() == 0)
		{
			Text += TEXT("No equip events yet.\n");
		}
		else
		{
			for (int32 Index = Events.Num() - 1; Index >= 0; --Index)
			{
				const FEquipmentDebugEvent& Event = Events[Index];
				Text += FString::Printf(TEXT("#%03d [%s] %s\n"),
					Event.Sequence,
					Event.bPassed ? TEXT("PASS") : TEXT("FAIL"),
					*Event.Message);
			}
		}
	}

	Text += TEXT("\n== Option Roll Stress ==\n");
	UItemSubsystem* ItemSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UItemSubsystem>() : nullptr;
	if (!IsValid(ItemSubsystem) || ItemSubsystem->GetLastOptionRollStressReport().IsEmpty())
	{
		Text += TEXT("No option roll report yet.\n");
	}
	else
	{
		Text += ItemSubsystem->GetLastOptionRollStressReport();
		if (!Text.EndsWith(TEXT("\n")))
		{
			Text += TEXT("\n");
		}
	}

	DebugText->SetText(FText::FromString(Text));
}
