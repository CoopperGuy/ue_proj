// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaInstance.h"
#include "Skill/DiaSkillManager.h"

void UDiaInstance::Init()
{
    Super::Init();

    // 매니저들 생성 및 초기화
    SkillManager = NewObject<UDiaSkillManager>(this);
    SkillManager->Initialize();

}
