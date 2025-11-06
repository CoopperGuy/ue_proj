// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/CharacterStatus/StatusWidget.h"
#include "Components/TextBlock.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/DiaAttributeSet.h"

void UStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// PlayerController와 Pawn 가져오기
	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) return;

	APawn* Pawn = PC->GetPawn();
	if (!IsValid(Pawn)) return;

	// AbilitySystemComponent 가져오기 및 캐싱
	CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
	if (!IsValid(CachedASC)) return;

	// Health & Mana
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetManaAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetMaxManaAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	
	// Experience
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetExpAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetMaxExpAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	
	// Base Stats
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetStrengthAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetDexterityAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetIntelligenceAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	
	// Combat Stats
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetAttackPowerAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);
	CachedASC->GetGameplayAttributeValueChangeDelegate(UDiaAttributeSet::GetDefenseAttribute())
		.AddUObject(this, &UStatusWidget::OnUpdateStats);

	// 초기 값 표시를 위해 모든 속성 한 번 업데이트
	const UDiaAttributeSet* AttributeSet = CachedASC->GetSet<UDiaAttributeSet>();
	if (AttributeSet)
	{
		// HP/MP
		if (HPText)
		{
			const float Health = AttributeSet->GetHealth();
			const float MaxHealth = AttributeSet->GetMaxHealth();
			HPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")),
				FText::AsNumber(FMath::FloorToInt(Health)),
				FText::AsNumber(FMath::FloorToInt(MaxHealth))));
		}
		if (MPText)
		{
			const float Mana = AttributeSet->GetMana();
			const float MaxMana = AttributeSet->GetMaxMana();
			MPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")),
				FText::AsNumber(FMath::FloorToInt(Mana)),
				FText::AsNumber(FMath::FloorToInt(MaxMana))));
		}
		
		// EXP
		if (ExpText)
		{
			const float Exp = AttributeSet->GetExp();
			const float MaxExp = AttributeSet->GetMaxExp();
			ExpText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")),
				FText::AsNumber(FMath::FloorToInt(Exp)),
				FText::AsNumber(FMath::FloorToInt(MaxExp))));
		}
		
		// Base Stats
		if (StrText)
		{
			StrText->SetText(FText::AsNumber(FMath::FloorToInt(AttributeSet->GetStrength())));
		}
		if (DexText)
		{
			DexText->SetText(FText::AsNumber(FMath::FloorToInt(AttributeSet->GetDexterity())));
		}
		if (IntText)
		{
			IntText->SetText(FText::AsNumber(FMath::FloorToInt(AttributeSet->GetIntelligence())));
		}
		
		// Combat Stats
		if (AtkText)
		{
			AtkText->SetText(FText::AsNumber(FMath::FloorToInt(AttributeSet->GetAttackPower())));
		}
		if (DefText)
		{
			DefText->SetText(FText::AsNumber(FMath::FloorToInt(AttributeSet->GetDefense())));
		}
	}
}

void UStatusWidget::OnUpdateStats(const FOnAttributeChangeData& Data)
{
	if (!CachedASC) return;

	const FString AttributeName = Data.Attribute.GetName();
	
	// Health 관련
	if (AttributeName == TEXT("Health") || AttributeName == TEXT("MaxHealth"))
	{
		if (HPText)
		{
			const float Health = CachedASC->GetNumericAttribute(UDiaAttributeSet::GetHealthAttribute());
			const float MaxHealth = CachedASC->GetNumericAttribute(UDiaAttributeSet::GetMaxHealthAttribute());
			HPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")),
				FText::AsNumber(FMath::FloorToInt(Health)),
				FText::AsNumber(FMath::FloorToInt(MaxHealth))));
		}
	}
	// Mana 관련
	else if (AttributeName == TEXT("Mana") || AttributeName == TEXT("MaxMana"))
	{
		if (MPText)
		{
			const float Mana = CachedASC->GetNumericAttribute(UDiaAttributeSet::GetManaAttribute());
			const float MaxMana = CachedASC->GetNumericAttribute(UDiaAttributeSet::GetMaxManaAttribute());
			MPText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")),
				FText::AsNumber(FMath::FloorToInt(Mana)),
				FText::AsNumber(FMath::FloorToInt(MaxMana))));
		}
	}
	// Experience 관련
	else if (AttributeName == TEXT("Exp") || AttributeName == TEXT("MaxExp"))
	{
		if (ExpText)
		{
			const float Exp = CachedASC->GetNumericAttribute(UDiaAttributeSet::GetExpAttribute());
			const float MaxExp = CachedASC->GetNumericAttribute(UDiaAttributeSet::GetMaxExpAttribute());
			ExpText->SetText(FText::Format(FText::FromString(TEXT("{0}/{1}")),
				FText::AsNumber(FMath::FloorToInt(Exp)),
				FText::AsNumber(FMath::FloorToInt(MaxExp))));
		}
	}
	// Base Stats
	else if (AttributeName == TEXT("Strength"))
	{
		if (StrText)
		{
			StrText->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue)));
		}
	}
	else if (AttributeName == TEXT("Dexterity"))
	{
		if (DexText)
		{
			DexText->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue)));
		}
	}
	else if (AttributeName == TEXT("Intelligence"))
	{
		if (IntText)
		{
			IntText->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue)));
		}
	}
	// Combat Stats
	else if (AttributeName == TEXT("AttackPower"))
	{
		if (AtkText)
		{
			AtkText->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue)));
		}
	}
	else if (AttributeName == TEXT("Defense"))
	{
		if (DefText)
		{
			DefText->SetText(FText::AsNumber(FMath::FloorToInt(Data.NewValue)));
		}
	}
}
