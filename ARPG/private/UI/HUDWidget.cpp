// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUDWidget.h"
#include "UI/Orb.h"
#include "UI/MonsterHP.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

//widget의 퍼센테이지만 조절한다
void UHUDWidget::UpdateOrbPercentage(OrbType _Type, float _Percentage)
{
	switch (_Type)
	{
	case OrbType::OT_HP:
		HpWidget->UpdatePercentage(_Percentage);
		break;
	case OrbType::OT_MP:
		MpWidget->UpdatePercentage(_Percentage);
		break;
	}
}

//widget의 퍼센테이지만 조절한다
//어떤 몬스터의 것을 업데이트 할 지는 다른 곳에서 처리하자.
//-> DiaCombatComponent 
void UHUDWidget::UpdateMonsterPercentage(BarType _Type, float _Percentage)
{
	switch (_Type)
	{
	case BarType::BT_HP:
		MonsterHPWidget->UpdatePercentage(_Percentage);
		break;
	}
}
