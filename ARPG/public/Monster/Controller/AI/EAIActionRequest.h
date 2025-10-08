// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EAIActionRequest.generated.h"


UENUM(BlueprintType)
enum class EAIActionRequest : uint8
{
	None	UMETA(DisplayName = "None"),
	Skill1	UMETA(DisplayName = "Skill1"),
};