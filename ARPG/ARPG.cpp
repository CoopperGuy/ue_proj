// Copyright Epic Games, Inc. All Rights Reserved.

#include "ARPG.h"
#include "Modules/ModuleManager.h"
#include "GAS/DiaGameplayTags.h"

class FARPGGameModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		// GameplayTag 초기화
		FDiaGameplayTags::InitializeNativeTags();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FARPGGameModule, ARPG, "ARPG" );
 