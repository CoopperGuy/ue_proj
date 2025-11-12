// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaInstance.h"
#include "Skill/DiaSkillManager.h"
#include "System/GameViewPort/DiaCustomGameViewPort.h"
#include "Engine/Engine.h"

void UDiaInstance::Init()
{
    Super::Init();

    // 스킬 매니저 생성 및 초기화
	//이건 UObject를 기반으로 만들어서 NewObject로 생성해야 함
    SkillManager = NewObject<UDiaSkillManager>(this);
    SkillManager->Initialize();
 
}
