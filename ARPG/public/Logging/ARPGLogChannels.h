#pragma once

#include "CoreMinimal.h"

class AActor;

// Runtime console examples:
// Log LogARPG_Skill VeryVerbose
// Log LogARPG_Inventory Warning
// Log list

DECLARE_LOG_CATEGORY_EXTERN(LogARPG_Inventory, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogARPG_GAS, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogARPG_Map, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogARPG_Room, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogARPG_Spawn, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogARPG_UI, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogARPGAttribute, Warning, All);

DECLARE_LOG_CATEGORY_EXTERN(LogARPG, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogARPG_Skill, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogARPG_Combat, Warning, All);
DECLARE_LOG_CATEGORY_EXTERN(LogARPG_AI, Warning, All);

namespace ARPGLog
{
	ARPG_API FString DescribeActor(const AActor* Actor);
}

#define ARPG_LOG(CategoryName, Verbosity, Format, ...) \
	UE_LOG(CategoryName, Verbosity, TEXT("[%s] ") Format, ANSI_TO_TCHAR(__FUNCTION__), ##__VA_ARGS__)

#define ARPG_SKILL_LOG(Verbosity, Format, ...) ARPG_LOG(LogARPG_Skill, Verbosity, Format, ##__VA_ARGS__)
#define ARPG_SKILL_VLOG(Format, ...) ARPG_SKILL_LOG(Verbose, Format, ##__VA_ARGS__)
#define ARPG_SKILL_VVLOG(Format, ...) ARPG_SKILL_LOG(VeryVerbose, Format, ##__VA_ARGS__)

#define ARPG_GAS_LOG(Verbosity, Format, ...) ARPG_LOG(LogARPG_GAS, Verbosity, Format, ##__VA_ARGS__)
#define ARPG_UI_LOG(Verbosity, Format, ...) ARPG_LOG(LogARPG_UI, Verbosity, Format, ##__VA_ARGS__)
#define ARPG_INVENTORY_LOG(Verbosity, Format, ...) ARPG_LOG(LogARPG_Inventory, Verbosity, Format, ##__VA_ARGS__)
