// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/DiaSkillVariant.h"
#include "Engine/GameInstance.h"
#include "System/GASSkillManager.h"
#include "Types/DiaGASSkillData.h"
UDiaSkillVariant::UDiaSkillVariant()
{
}

void UDiaSkillVariant::InitializeVariant(const FDiaSkillVariantSpec& spec)
{
	VariantSpec = spec;

}

