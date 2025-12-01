// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DiaSkillDataTable.generated.h"

class ADiaSkillBase;
USTRUCT(BlueprintType)
struct ARPG_API FDiaSkillTableRow : public FTableRowBase
{
    GENERATED_BODY()

    // ��ų �⺻ ����
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText SkillDescription;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UTexture2D* SkillIcon = nullptr;

    // ��ų ����
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Cooldown = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float ManaCost = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float Damage = 0.0f;

    // ��ų Ŭ���� (��������Ʈ�� C++ Ŭ���� ���� ����)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<ADiaSkillBase> SkillClass;
};
/**
 * 
 */
UCLASS()
class ARPG_API UDiaSkillDataTable : public UDataTable
{
	GENERATED_BODY()
	
	
	
	
};
