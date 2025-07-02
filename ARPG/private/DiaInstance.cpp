// Fill out your copyright notice in the Description page of Project Settings.


#include "DiaInstance.h"
#include "Skill/DiaSkillManager.h"

void UDiaInstance::Init()
{
    Super::Init();

    // �Ŵ����� ���� �� �ʱ�ȭ
    SkillManager = NewObject<UDiaSkillManager>(this);
    SkillManager->Initialize();

}
