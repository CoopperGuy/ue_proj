// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUDWidget.h"
#include "UI/Orb.h"
#include "UI/MonsterHP.h"

void UHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

//widget�� �ۼ��������� �����Ѵ�
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

//widget�� �ۼ��������� �����Ѵ�
//� ������ ���� ������Ʈ �� ���� �ٸ� ������ ó������.
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
