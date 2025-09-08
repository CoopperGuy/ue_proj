#include "GAS/DiaGASDebugHelper.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "GAS/DiaAttributeSet.h"
#include "Engine/Engine.h"

void UDiaGASDebugHelper::CheckASCOnCharacter(AActor* Character)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("CheckASCOnCharacter: Character is null"));
		return;
	}

	if (UAbilitySystemComponent* ASC = Character->FindComponentByClass<UAbilitySystemComponent>())
	{
		UE_LOG(LogTemp, Log, TEXT("ASC found on %s"), *Character->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ASC NOT found on %s"), *Character->GetName());
	}
}

void UDiaGASDebugHelper::PrintAttributeValues(AActor* Character)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("PrintAttributeValues: Character is null"));
		return;
	}

	if (UAbilitySystemComponent* ASC = Character->FindComponentByClass<UAbilitySystemComponent>())
	{
		if (const UDiaAttributeSet* Attr = Cast<UDiaAttributeSet>(ASC->GetAttributeSet(UDiaAttributeSet::StaticClass())))
		{
			UE_LOG(LogTemp, Log, TEXT("Health=%.1f/%.1f, Mana=%.1f/%.1f, Attack=%.1f, Defense=%.1f, MoveSpeed=%.1f"),
				Attr->GetHealth(), Attr->GetMaxHealth(),
				Attr->GetMana(), Attr->GetMaxMana(),
				Attr->GetAttackPower(), Attr->GetDefense(),
				Attr->GetMovementSpeed());
		}
	}
}

void UDiaGASDebugHelper::ForceInitializeASC(AActor* Character)
{
	if (UAbilitySystemComponent* ASC = Character ? Character->FindComponentByClass<UAbilitySystemComponent>() : nullptr)
	{
		ASC->InitAbilityActorInfo(Character, Character);
		UE_LOG(LogTemp, Log, TEXT("ASC initialized for %s"), *Character->GetName());
	}
}

void UDiaGASDebugHelper::CheckGASPluginStatus()
{
	UE_LOG(LogTemp, Log, TEXT("GAS plugin status: assumed enabled (runtime check stub)"));
}