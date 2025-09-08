#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DiaGASDebugHelper.generated.h"

class UAbilitySystemComponent;
class UDiaAttributeSet;

UCLASS()
class ARPG_API UDiaGASDebugHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Debug: Check if ASC exists on character
	UFUNCTION(BlueprintCallable, Category = "GAS|Debug")
	static void CheckASCOnCharacter(AActor* Character);

	// Debug: Print all attribute values
	UFUNCTION(BlueprintCallable, Category = "GAS|Debug")
	static void PrintAttributeValues(AActor* Character);

	// Debug: Force initialize ASC
	UFUNCTION(BlueprintCallable, Category = "GAS|Debug")
	static void ForceInitializeASC(AActor* Character);

	// Debug: Check GAS plugin status
	UFUNCTION(BlueprintCallable, Category = "GAS|Debug")
	static void CheckGASPluginStatus();
};