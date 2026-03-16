// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaInstance.h"
#include "Skill/DiaSkillManager.h"
#include "System/GameViewPort/DiaCustomGameViewPort.h"
#include "Engine/Engine.h"

void UDiaInstance::Init()
{
    Super::Init(); 
}

void UDiaInstance::OnStart()
{
    Super::OnStart();
    // ViewPort에서 DiaPrimaryLayout 초기화 요청
    if (UDiaCustomGameViewPort* VP = Cast<UDiaCustomGameViewPort>(GetGameViewportClient()))
    {
        VP->SetupGameInstance(this);
    }
}
