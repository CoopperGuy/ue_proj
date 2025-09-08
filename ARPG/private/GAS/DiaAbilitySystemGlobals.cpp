#include "GAS/DiaAbilitySystemGlobals.h"

UDiaAbilitySystemGlobals::UDiaAbilitySystemGlobals()
{
}

void UDiaAbilitySystemGlobals::InitGlobalData()
{
	Super::InitGlobalData();
	
	UE_LOG(LogTemp, Log, TEXT("DiaAbilitySystemGlobals initialized"));
}