// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaComponent/Skill/SkillObject.h"
#include "DiaComponent/Skill/DiaSkillVariant.h"

#include "GAS/DiaGameplayAbility.h"
#include "GAS/Abilities/DiaProjectileAbility.h"
#include "GAS/Abilities/DiaGroundAbility.h"

#include "Engine/GameInstance.h"
#include "System/GASSkillManager.h"

void USkillObject::InitializeSkillObject(int32 _SkillID)
{
	SkillID = _SkillID;
}

void USkillObject::SetVariantApplyIDs(int32 _AddVariantID)
{
	VariantApplyIDs.Add(_AddVariantID);
}

const TSet<int32>& USkillObject::GetVariantApplyIDs() const
{
    // TODO: 여기에 return 문을 삽입합니다.
	UE_LOG(LogTemp, Warning, TEXT("USkillObject::GetVariantApplyIDs - Returning VariantApplyIDs, ID Size : %d"), VariantApplyIDs.Num());
    return VariantApplyIDs;
}

const FGASSkillData* USkillObject::GetSkillData() const
{
    UGASSkillManager* GasSkillMgr = GetWorld()->GetGameInstance() ? GetWorld()->GetGameInstance()->GetSubsystem<UGASSkillManager>() : nullptr;
    if (!GasSkillMgr)
    {
        UE_LOG(LogTemp, Warning, TEXT("GrantInitialGASAbilities: No GASSkillManager"));
    }

    return GasSkillMgr->GetSkillDataPtr(SkillID);
}
