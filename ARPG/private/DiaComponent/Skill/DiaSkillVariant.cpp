// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "Engine/GameInstance.h"
#include "System/GASSkillManager.h"
#include "Types/DiaGASSkillData.h"
UDiaSkillVariant::UDiaSkillVariant()
{
}

void UDiaSkillVariant::InitializeVariant(int32 _VariantID)
{
	UGASSkillManager* GasSkillMgr = GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
	if (!GasSkillMgr)
	{
		UE_LOG(LogTemp, Warning, TEXT("GrantInitialGASAbilities: No GASSkillManager"));
	}
	const FSkillVariantData* Data = GasSkillMgr->GetSkllVariantDataPtr(_VariantID);
	if (Data)
	{
		SkillTag = Data->VariantTag;
		ModifierValue = Data->ModifierValue;
	}
}

void UDiaSkillVariant::SkillVariantFunction()
{

}

void UDiaSkillVariant::SkillSpawnFunction()
{
}

void UDiaSkillVariant::SkillHitFunction()
{
}

void UDiaSkillVariant::SkillEndFunction()
{
}
